#pragma once

#include <BASIS/buffer.h>
#include <BASIS/texture.h>
#include <BASIS/manager.h>
#include <BASIS/pipeline.h>
#include <BASIS/context.h>

#include <span>
#include <cstdint>

#include <glm/mat4x4.hpp>

namespace BASIS
{
// main structure responsible for rendering
// all static functions influence only global gl context and don't need checks
// non static function introduce some context changes and/or check context flags
// should be used once per app
struct Renderer
{
	std::unique_ptr<RenderingContext> context = std::make_unique<RenderingContext>();
	void beginFrame();
	void endFrame();
	void beginCompute();
	void endCompute();

	
	// remove this later and give user ability to interpret model data however he wants
	void drawModel(const std::shared_ptr<GLTFModel>& model);
	
	void bindPipeline(const Pipeline& pipeline);
	void bindComputePipeline(const ComputePipeline& pipe);
	void bindIndexBuffer(const Buffer& buffer,IndexType type = IndexType::UINT);
	
	void bindVertexBuffer(
		const Buffer& buffer, 
		std::uint32_t bindPoint,
		std::uint64_t stride = sizeof(Vertex),
		std::uint64_t offs = 0);
		
	void draw(
		std::uint32_t vertexCount,
		std::uint32_t vertexOffset = 0,
		std::uint32_t instanceCount = 1,
		std::uint32_t firstInstance = 0);
		
	void drawIndexed(
		std::uint32_t idxCount,
		std::uint32_t idxOffset = 0,
		std::int32_t  vertOffset = 0,
		std::uint32_t instanceCount = 1,
		std::uint32_t firstInstance = 0);
	
	void drawIndirect(
		const Buffer& cmdBuf,
		std::uint32_t drawCount,
		std::uint32_t stride,
		std::uint64_t bufOffset = 0);
					 
	void bindUniformBuffer(
		const Buffer& buf,
		std::uint32_t idx,
		std::uint64_t size = WHOLE_BUFFER,
		std::uint64_t offs = 0);
	
	void bindStorageBuffer(
		const Buffer& buf,
		std::uint32_t idx,
		std::uint64_t size = WHOLE_BUFFER,
		std::uint64_t offs = 0);
	
	void drawIndirectCount(
		const Buffer& commandBuffer,
		const Buffer& countBuffer,
		std::uint32_t maxDrawCount,
		std::uint32_t stride,
		std::uint64_t commandBufferOffset=0,
		std::uint64_t countBufferOffset=0);

	void drawIndexedIndirect(
		const Buffer& commandBuffer,
		std::uint32_t drawCount,
		std::uint32_t stride,
		std::uint64_t commandBufferOffset = 0);
	
	void drawIndexedIndirectCount(
	const Buffer& commandBuffer,
	const Buffer& countBuffer,
	std::uint32_t maxDrawCount,
	std::uint32_t stride,
	std::uint64_t commandBufferOffset = 0,
	std::uint64_t countBufferOffset = 0);
	
	void bindSampledImage(std::uint32_t index, const Texture& texture, const Sampler& sampler);
	
	void clearColor(float r,float g,float b,float a);
	void clear(MaskFlags mask);
	
	static void enableCapability(Cap capability);
	static void disableCapability(Cap capability);
	
	static void cullFace(CullFaceMode mode);
	static void frontFace(FrontFaceMode mode);
	
	static void blendFunc(Factor src,Factor dst);
	static void blendFuncSeparate(Factor srcRGB,Factor dstRGB,Factor srcAlpha,Factor dstAlpha);
	
	static void setUniform(
		std::uint8_t size,
		std::int32_t location,
		std::size_t count,
		const std::int32_t* val);
		
	static void setUniform(
		std::uint8_t size,
		std::int32_t location,
		std::size_t count,
		const std::uint32_t* val);
		
	static void setUniform(
		FloatUniform type,
		std::int32_t location,
		std::size_t count,
		const float* val,
		bool transpose=false);
	
	
};
	
}
