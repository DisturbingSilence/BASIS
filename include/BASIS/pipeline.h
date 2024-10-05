#pragma once
#include <span>
#include <vector>
#include <cstdint>
#include <string_view>

namespace BASIS
{
	
struct Buffer;
struct Shader : public BaseClass
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

struct InputAssemblyState
{
	PrimitiveMode mode = PrimitiveMode::TRIANGLES;
	bool primitiveRestartEnable = false;
};
using VertexInputState = std::vector<VertexBinding>;

struct PipelineInfo
{
	InputAssemblyState assemblyState{};
	std::vector<VertexBinding> vertexState{};
};
struct PipelineCreateInfo : public PipelineInfo
{
	const Shader* vertex{};
	const Shader* fragment{};
	const Shader* tesselationControl{};
	const Shader* tesselationEvaluation{};
};
struct Pipeline : public BaseClass
{
	explicit Pipeline(const PipelineCreateInfo& info,std::string_view name="");
	~Pipeline();
	
	Pipeline(Pipeline&&) noexcept;
	Pipeline& operator=(Pipeline&&) noexcept;
	
	private:
	PipelineInfo m_info{};
	friend class Renderer;
};
	
	
}
