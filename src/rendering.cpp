#include <BASIS/types.h>
#include <BASIS/context.h>
#include <BASIS/pipeline.h>
#include <BASIS/rendering.h>

#include <cassert>
#include <algorithm>

#include <glad/gl.h>

namespace
{
static std::size_t hashVAO(const BASIS::VertexInputState& state)
{
	std::size_t totalHash{};
	std::for_each(state.begin(),state.end(),
	[&](const auto& cur)
	{
		auto tup = std::make_tuple(cur.location, cur.binding, cur.fmt, cur.offset);
		auto tupHash = BASIS::hash<decltype(tup)>{}(tup);
		BASIS::hash_combine(totalHash,tupHash);
	});
	return totalHash;
}
std::unordered_map<std::size_t,std::uint32_t> vaoCache;
static std::uint32_t getVAO(const BASIS::VertexInputState& state)
{
	auto vao_hash = hashVAO(state);
	if(auto it = vaoCache.find(vao_hash);it != vaoCache.end()) return it->second;
	
	std::uint32_t id{};
	glCreateVertexArrays(1, &id);
	std::for_each(state.begin(),state.end(),
	[&](const auto& cur)
	{
		glEnableVertexArrayAttrib(id, cur.location);
		glVertexArrayAttribBinding(id, cur.location, cur.binding);
		glVertexArrayAttribFormat(id, cur.location,
		formatTo(cur.fmt,BASIS::BITMASK::SIZE_GL),
		formatTo(cur.fmt,BASIS::BITMASK::TYPE_GL),
		formatTo(cur.fmt,BASIS::BITMASK::IS_NORMALIZED),
		cur.offset);
	});
	return vaoCache.try_emplace(vao_hash,id).first->second;	
}
static void enableOrDisable(std::uint32_t state, bool value)
{
	value ? glEnable(state) : glDisable(state);
}
}
namespace BASIS
{
	
void Renderer::bindUniformBuffer(const Buffer& buf,std::uint32_t idx,std::uint64_t size,std::uint64_t offs)
{
	assert(context->isRendering);
	glBindBufferRange(GL_UNIFORM_BUFFER, idx, 
	buf.id(), offs, 
	size == WHOLE_BUFFER ? buf.size() - offs : size);
}
void Renderer::bindStorageBuffer(const Buffer& buf,std::uint32_t idx,std::uint64_t size,std::uint64_t offs)
{
	assert(context->isRendering);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, idx, 
	buf.id(), offs, 
	size == WHOLE_BUFFER ? buf.size() - offs : size);
}
void Renderer::bindSampledImage(std::uint32_t index, const Texture& texture, const Sampler& sampler)
{
	assert(context->isRendering);
	glBindTextureUnit(index, texture.id());
    glBindSampler(index, sampler.id());
}
void Renderer::bindPipeline(const Pipeline& pipe)
{
	assert(context->isRendering);
	assert(pipe.id() && "Can't bind uninitialized pipeline");
	
	if(context->lastPipelineInfo == pipe.info()) return;
	glUseProgram(pipe.id());

	const auto& inf = pipe.info();
	
	context->primitiveMode = inf->mode;
	if(auto newVao = getVAO(inf->vertexInputState);newVao != context->vao)
	{
		context->vao = newVao;
		glBindVertexArray(newVao);
	}
	// TessellationState
	const auto& tss = inf->tessellationState;
	if(tss.patchVertices > 0)
	{
		if(!context->lastPipelineInfo || tss.patchVertices != context->lastPipelineInfo->tessellationState.patchVertices)
		{
			glPatchParameteri(GL_PATCH_VERTICES, tss.patchVertices);
		}
	}
	if(!context->lastPipelineInfo || tss.innerPatchLevel != context->lastPipelineInfo->tessellationState.innerPatchLevel)
	{
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,tss.innerPatchLevel.data());
	}
	if(!context->lastPipelineInfo || tss.outerPatchLevel != context->lastPipelineInfo->tessellationState.outerPatchLevel)
	{
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,tss.outerPatchLevel.data());
	}

	// DepthState
	const auto& ds = inf->depthState;
	if (!context->lastPipelineInfo || ds.depthTestEnable != context->lastPipelineInfo->depthState.depthTestEnable)
	{
		enableOrDisable(GL_DEPTH_TEST, ds.depthTestEnable);
	}

	if (!context->lastPipelineInfo || ds.depthWriteEnable != context->lastPipelineInfo->depthState.depthWriteEnable)
	{
		glDepthMask(ds.depthWriteEnable);
	}

	if (!context->lastPipelineInfo || ds.depthCompareOp != context->lastPipelineInfo->depthState.depthCompareOp)
	{
		glDepthFunc(enumToGL(ds.depthCompareOp));
	}

	const auto& rs = inf->rasterizationState;
	if (!context->lastPipelineInfo || rs.depthClampEnable != context->lastPipelineInfo->rasterizationState.depthClampEnable)
	{
		enableOrDisable(GL_DEPTH_CLAMP, rs.depthClampEnable);
	}

	if (!context->lastPipelineInfo || rs.polygonMode != context->lastPipelineInfo->rasterizationState.polygonMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, enumToGL(rs.polygonMode));
	}

	if (!context->lastPipelineInfo || rs.cullMode != context->lastPipelineInfo->rasterizationState.cullMode)
	{
		enableOrDisable(GL_CULL_FACE, rs.cullMode != CullMode::NONE);
		if (rs.cullMode != CullMode::NONE)
		{
			glCullFace(enumToGL(rs.cullMode));
		}
	}

	if (!context->lastPipelineInfo || rs.frontFace != context->lastPipelineInfo->rasterizationState.frontFace)
	{
		glFrontFace(enumToGL(rs.frontFace));
	}

	if (!context->lastPipelineInfo || rs.depthBiasEnable != context->lastPipelineInfo->rasterizationState.depthBiasEnable)
	{
		enableOrDisable(GL_POLYGON_OFFSET_FILL, rs.depthBiasEnable);
		enableOrDisable(GL_POLYGON_OFFSET_LINE, rs.depthBiasEnable);
		enableOrDisable(GL_POLYGON_OFFSET_POINT, rs.depthBiasEnable);
	}

	if (!context->lastPipelineInfo ||
	rs.depthBiasSlopeFactor != context->lastPipelineInfo->rasterizationState.depthBiasSlopeFactor ||
	rs.depthBiasConstantFactor != context->lastPipelineInfo->rasterizationState.depthBiasConstantFactor)
	{
		glPolygonOffset(rs.depthBiasSlopeFactor, rs.depthBiasConstantFactor);
	}

	if (!context->lastPipelineInfo || rs.lineWidth != context->lastPipelineInfo->rasterizationState.lineWidth)
	{
		glLineWidth(rs.lineWidth);
	}

	if (!context->lastPipelineInfo || rs.pointSize != context->lastPipelineInfo->rasterizationState.pointSize)
	{
		glPointSize(rs.pointSize);
	}

}
void Renderer::blitFramebuffer(
	const Framebuffer& src,
	const Framebuffer& dst,
	glm::ivec4 srcRect,
	glm::ivec4 dstRect,
	MaskFlags mask,
	Filter filter)
{
	assert(filter == Filter::NEAREST || filter == Filter::LINEAR);
	glBlitNamedFramebuffer(
		src.id(),dst.id(),
		srcRect.x,srcRect.y,srcRect.z,srcRect.w,
		dstRect.x,dstRect.y,dstRect.z,dstRect.w,
		static_cast<std::uint32_t>(mask),
		static_cast<std::uint32_t>(filter));
}

void Renderer::bindFramebuffer(const Framebuffer& fbo)
{
	if(fbo.id() == context->fbo) return;
	context->fbo = fbo.id();
	glBindFramebuffer(GL_FRAMEBUFFER,fbo.id());
}
void Renderer::bindDefaultFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
}
bool Renderer::isValidDrawFramebuffer(const Framebuffer& fb)
{
	return glCheckNamedFramebufferStatus(fb.id(),GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
bool Renderer::isValidReadFramebuffer(const Framebuffer& fb)
{
	return glCheckNamedFramebufferStatus(fb.id(),GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}
void Renderer::bindComputePipeline(const ComputePipeline& pipe)
{
	assert(context->isComputeActive);
	assert(pipe.id());

	if(context->lastBoundPipeline == pipe.id()) return;
	glUseProgram(pipe.id());

}
void Renderer::bindIndexBuffer(const Buffer& buf,IndexType type)
{
	assert(context->isRendering);
	context->isIdxBufferBound = true;
	context->idxType = type;
	glVertexArrayElementBuffer(context->vao, buf.id());
}
void Renderer::bindVertexBuffer(const Buffer& buf,std::uint32_t bindPoint,std::uint64_t stride,std::uint64_t offs)
{
	assert(context->isRendering);
	glVertexArrayVertexBuffer(context->vao, bindPoint, buf.id(), offs, stride);
}
void Renderer::beginFrame()
{
	assert(!context->isRendering && "Cannot call BeginFrame() twice");
	context->isRendering = true;
}
void Renderer::endFrame()
{
	assert(context->isRendering && "Cannot call EndFrame() without rendering");
	context->isRendering = false;
	context->isIdxBufferBound = false;
}
void Renderer::draw(
		std::uint32_t vertexCount,
		std::uint32_t vertexOffset,
		std::uint32_t instanceCount,
		std::uint32_t firstInstance)	
{
	assert(context->isRendering);
	glDrawArraysInstancedBaseInstance(
		enumToGL(context->primitiveMode),
		vertexOffset,
		vertexCount,
		instanceCount,
		firstInstance);	
}
	
void Renderer::drawIndirect(
	const Buffer& commandBuffer,
	std::uint32_t drawCount,
	std::uint32_t stride,
	std::uint64_t bufOffset)
{
	assert(context->isRendering);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.id());
	glMultiDrawArraysIndirect(enumToGL(context->primitiveMode),
	reinterpret_cast<void*>(static_cast<uintptr_t>(bufOffset)),
	drawCount,
	stride);
}
void Renderer::drawIndexed(
	std::uint32_t idxCount,
	std::uint32_t idxOffset,
	std::int32_t  vertOffset,
	std::uint32_t instanceCount,
	std::uint32_t firstInstance)
{
	assert(context->isRendering);
	assert(context->isIdxBufferBound);
	glDrawElementsInstancedBaseVertexBaseInstance(
		enumToGL(context->primitiveMode),
		idxCount,
		enumToGL(context->idxType),
		reinterpret_cast<void*>(idxOffset * sizeof(context->idxType)),
		instanceCount,
		vertOffset,
		firstInstance);
}
void Renderer::drawIndirectCount(
	const Buffer& commandBuffer,
	const Buffer& countBuffer,
	std::uint32_t maxDrawCount,
	std::uint32_t stride,
	std::uint64_t commandBufferOffset,
	std::uint64_t countBufferOffset)
{
	assert(context->isRendering);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.id());
	glBindBuffer(GL_PARAMETER_BUFFER, countBuffer.id());
	glMultiDrawArraysIndirectCount(enumToGL(context->primitiveMode),
	reinterpret_cast<void*>(static_cast<std::uintptr_t>(commandBufferOffset)),
	static_cast<int32_t>(countBufferOffset),
	maxDrawCount,
	stride);
}



void Renderer::drawIndexedIndirect(
	const Buffer& commandBuffer,
	std::uint32_t drawCount,
	std::uint32_t stride,
	std::uint64_t commandBufferOffset)
{
	assert(context->isRendering);
	assert(context->isIndexBufferBound);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.id());
	glMultiDrawElementsIndirect(enumToGL(context->primitiveMode),
	enumToGL(context->idxType),
	reinterpret_cast<void*>(static_cast<std::uintptr_t>(commandBufferOffset)),
	drawCount,
	stride);
}
void Renderer::drawIndexedIndirectCount(
	const Buffer& commandBuffer,
	const Buffer& countBuffer,
	std::uint32_t maxDrawCount,
	std::uint32_t stride,
	std::uint64_t commandBufferOffset,
	std::uint64_t countBufferOffset)
{
	assert(context->isRendering);
	assert(context->isIndexBufferBound);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer.id());
	glBindBuffer(GL_PARAMETER_BUFFER, countBuffer.id());
	glMultiDrawElementsIndirectCount(enumToGL(context->idxType),
	enumToGL(context->idxType),
	reinterpret_cast<void*>(static_cast<std::uintptr_t>(commandBufferOffset)),
	static_cast<std::int32_t>(countBufferOffset),
	maxDrawCount,
	stride);
}

void Renderer::clearColor(float r,float g,float b,float a)
{
	glClearColor(r,g,b,a);
}
void Renderer::clear(MaskFlags mask)
{
	assert(context->isRendering);
	glClear(static_cast<std::uint32_t>(mask));
}
void Renderer::enableCapability(Cap capability)
{
	glEnable(enumToGL(capability));
}
void Renderer::disableCapability(Cap capability)
{
	glDisable(enumToGL(capability));
}
void Renderer::dispatch(const glm::vec3& groupCount)
{
	assert(constext->isComputeActive);
	glDispatchCompute(groupCount.x, groupCount.y,groupCount.z);
}
void Renderer::dispatchIndirect(const Buffer& cmdBuf,std::uint64_t offset)
{
	assert(context->isComputeActive);

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, cmdBuf.id());
	glDispatchComputeIndirect(offset);
}

void Renderer::beginCompute()
{
	assert(!context->isComputeActive);
	assert(!context->isRendering);

	context->isComputeActive = true;
}
void Renderer::endCompute()
{
	assert(context->isComputeActive);
	context->isComputeActive = false;
}


void Renderer::setUniform(std::uint8_t size,std::int32_t location,std::size_t count,const std::int32_t* value)
{
	switch(size)
	{
		case 1:
			glUniform1iv(location,count,value);
			break;
		case 2:
			glUniform2iv(location,count,value);
			break;
		case 3:
			glUniform3iv(location,count,value);
			break;
		case 4:
			glUniform4iv(location,count,value);
			break;
		default:
			assert("invalid int32_t uniform size type");
			break;
	};
}
void Renderer::setUniform(std::uint8_t size,std::int32_t location,std::size_t count,const std::uint32_t* value)
{
	switch(size)
	{
		case 1:
			glUniform1uiv(location,count,value);
			break;
		case 2:
			glUniform2uiv(location,count,value);
			break;
		case 3:
			glUniform3uiv(location,count,value);
			break;
		case 4:
			glUniform4uiv(location,count,value);
			break;
		default:
			assert("invalid uint32_t uniform size type");
			break;
	};
}
void Renderer::setUniform(FloatUniform type,std::int32_t location,std::size_t count,const float* value,bool transpose)
{
	using enum FloatUniform;
	switch(type)
	{
		case f1:
			glUniform1fv(location,count,value);
			break;
		case f2:
			glUniform2fv(location,count,value);
			break;
		case f3:
			glUniform3fv(location,count,value);
			break;
		case f4:
			glUniform4fv(location,count,value);
			break;
		case mat2:
			glUniformMatrix2fv(location,count,transpose,value);
			break;
		case mat3:
			glUniformMatrix3fv(location,count,transpose,value);
			break;
		case mat4:
			glUniformMatrix4fv(location,count,transpose,value);
			break;
		case mat2x3:
			glUniformMatrix2x3fv(location,count,transpose,value);
			break;
		case mat3x2:
			glUniformMatrix3x2fv(location,count,transpose,value);
			break;
		case mat2x4:
			glUniformMatrix2x4fv(location,count,transpose,value);
			break;
		case mat4x2:
			glUniformMatrix4x2fv(location,count,transpose,value);
			break;
		case mat3x4:
			glUniformMatrix3x4fv(location,count,transpose,value);
			break;
		case mat4x3:
			glUniformMatrix4x3fv(location,count,transpose,value);
			break;
		default:
			assert("invalid float uniform size type");
	
	}
}
void Renderer::blendFunc(Factor src,Factor dst)
{
	glBlendFunc(enumToGL(src),enumToGL(dst));
}
void Renderer::blendFuncSeparate(Factor srcRGB,Factor dstRGB,Factor srcAlpha,Factor dstAlpha)
{
	glBlendFuncSeparate(enumToGL(srcRGB),enumToGL(dstRGB),enumToGL(srcAlpha),enumToGL(dstAlpha));
}

}
