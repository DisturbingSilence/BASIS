#pragma once

#include <BASIS/interfaces.h>

#include <span>
#include <array>
#include <memory>
#include <vector>
#include <cstdint>
#include <string_view>

namespace BASIS
{
	
struct Buffer;
struct Shader : public IGLObject
{
	explicit Shader(ShaderType type,std::string_view src,std::string_view name="");

	Shader(Shader&&) noexcept;
	Shader& operator=(Shader&&) noexcept;
	~Shader();
};
struct VertexBinding
{
	std::uint32_t 	location{};
	std::uint32_t 	binding{};
	std::uint32_t 	offset{};
	Format			fmt{};
};

struct TessellationState
{
	std::array<float,2> innerPatchLevel = {1.f,1.f};		// glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,);
	std::array<float,4> outerPatchLevel = {1.f,1.f,1.f,1.f};// glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,);

	std::uint32_t patchVertices{}; // glPatchParameteri(GL_PATCH_VERTICES,);
};
struct RasterizationState
{
	bool depthClampEnable         = false;
	PolygonMode polygonMode    	  = PolygonMode::FILL;
	CullMode cullMode             = CullMode::BACK;
	FrontFace frontFace           = FrontFace::CCW;
	bool depthBiasEnable          = false;
	float depthBiasConstantFactor = 0;
	float depthBiasSlopeFactor    = 0;
	float lineWidth               = 1; // glLineWidth
	float pointSize               = 1; // glPointSize
};
struct DepthState 
{
	bool depthTestEnable		= false;
	bool depthWriteEnable		= false;
	CompareMode depthCompareOp	= CompareMode::LESS;
};

using VertexInputState = std::vector<VertexBinding>;
struct PipelineInfo
{
	PrimitiveMode mode{PrimitiveMode::TRIANGLES};

	DepthState			depthState{};
	VertexInputState	vertexInputState{};
	TessellationState	tessellationState{};
	RasterizationState	rasterizationState{};

};
struct PipelineCreateInfo : public PipelineInfo
{
	const Shader* vertex{};
	const Shader* fragment{};
	const Shader* tesselationControl{};
	const Shader* tesselationEvaluation{};
};
struct Pipeline : public IGLObject
{
	explicit Pipeline(const PipelineCreateInfo& info,std::string_view name="");
	~Pipeline();
	
	Pipeline(Pipeline&&) noexcept;
	Pipeline& operator=(Pipeline&&) noexcept;
	
	// pulls info from internal pipeline cache
	const std::shared_ptr<const PipelineInfo> info() const noexcept;

};
struct ComputePipeline : public IGLObject
{
	explicit ComputePipeline(const Shader& computeShader,std::string_view name="");
	~ComputePipeline();
	
	ComputePipeline(ComputePipeline&&) noexcept;
	ComputePipeline& operator=(ComputePipeline&&) noexcept;
};
}
