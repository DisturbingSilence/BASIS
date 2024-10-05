#include <BASIS/types.h>
#include <BASIS/context.h>
#include <BASIS/pipeline.h>
#include <BASIS/rendering.h>

#include <cassert>
#include <algorithm>

#include <glad/gl.h>


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
	
	const auto& inf = pipe.info();
	if(context->lastBoundPipeline == pipe.id()) return;
	
	glUseProgram(pipe.id());
	
	context->primitiveMode = inf.mode;
	if(auto newVao = getVAO(inf.vertexState);newVao != context->vao)
	{
		context->vao = newVao;
		glBindVertexArray(newVao);
	}
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
void Renderer::cullFace(CullFaceMode mode)
{
	glCullFace(static_cast<std::uint32_t>(mode));
}
void Renderer::frontFace(FrontFaceMode mode)
{
	glFrontFace(static_cast<std::uint32_t>(mode));
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
