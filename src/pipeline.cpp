#include <BASIS/types.h>
#include <BASIS/pipeline.h>
#include <BASIS/exception.h>

#include <tuple>
#include <string>
#include <utility>
#include <cassert>
//#include <algorithm>
#include <unordered_map>

#include <glad/gl.h>

namespace
{
static bool link(unsigned int program,std::string& log)
{
	glLinkProgram(program);
	int success{};
	glGetProgramiv(program,GL_LINK_STATUS,&success);
	if(!success)
	{
		int len{512};
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		log.resize(len+1,'\0');
		glGetProgramInfoLog(program, len, nullptr, log.data());
		return false;
	}
	return true;
}
std::unordered_map<std::uint32_t,std::shared_ptr<const BASIS::PipelineInfo>> pipelineCache;
std::uint32_t compilePipeline(const BASIS::PipelineCreateInfo& info,std::string_view name)
{
	assert(info.vertex && "Pipeline must have at least vertex shader");
	assert(((info.tesselationControl != nullptr) == (info.tesselationEvaluation != nullptr)) && "Tesselation control and evalution go in pair" );
	
	auto m_id = glCreateProgram();
	glObjectLabel(GL_PROGRAM,m_id,name.size(),name.data());
	glAttachShader(m_id,info.vertex->id());
	
	if(info.fragment)				glAttachShader(m_id,info.fragment->id());
	if(info.tesselationControl)		glAttachShader(m_id,info.tesselationControl->id());
	if(info.tesselationEvaluation)	glAttachShader(m_id,info.tesselationEvaluation->id());
	std::string log;
	if(!link(m_id,log))
	{
		glDeleteProgram(m_id);
		throw BASIS::PipelineException("[LINKING FAILURE]\n",name,"\n",log);
	}
	return pipelineCache.insert({m_id,std::make_shared<BASIS::PipelineInfo>(info)}).first->first;
}
}
namespace BASIS
{
Shader::Shader(ShaderType type,std::string_view src,std::string_view name)
{
	auto s = src.data();
	m_id = glCreateShader(enumToGL(type));
	glObjectLabel(GL_SHADER,m_id,name.size(),name.data());
	glShaderSource(m_id,1,&s,nullptr);
	glCompileShader(m_id);
	
	int success{};
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		int len = 512;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &len);
		std::string log = std::string(len + 1, '\0');
		glGetShaderInfoLog(m_id, len, nullptr, log.data());
		glDeleteShader(m_id);
		throw PipelineException("[COMPILATION FAILURE]\n",name,"\n",log);
	}
	
}
Shader::Shader(Shader&& other) noexcept
{
	m_id = std::exchange(other.m_id,0);
}	

Shader& Shader::operator=(Shader&& other) noexcept
{
	if(&other == this) return *this;
	m_id = std::exchange(other.m_id,0);
	return *this;
}
Shader::~Shader()
{
	glDeleteShader(m_id);
}
Pipeline::Pipeline(const PipelineCreateInfo& info,std::string_view name)
{
	assert(info.vertex && "Pipeline must have at least vertex shader");
	
	assert(((info.tesselationControl != nullptr) == (info.tesselationEvaluation != nullptr)) && "Tesselation control and evalution go in pair" );
	
	m_id = glCreateProgram();
	glObjectLabel(GL_PROGRAM,m_id,name.size(),name.data());
	glAttachShader(m_id,info.vertex->id());
	
	if(info.fragment)				glAttachShader(m_id,info.fragment->id());
	if(info.tesselationControl)		glAttachShader(m_id,info.tesselationControl->id());
	if(info.tesselationEvaluation)	glAttachShader(m_id,info.tesselationEvaluation->id());
	std::string log;
	if(!link(m_id,log))
	{
		glDeleteProgram(m_id);
		throw PipelineException("[LINKING FAILURE]\n",name,"\n",log);
	}
	m_id = compilePipeline(info,name);
}
const std::shared_ptr<const PipelineInfo> Pipeline::info() const noexcept
{
    auto it = pipelineCache.find(m_id);
	return it != pipelineCache.end() ? it->second : nullptr;
}
Pipeline::~Pipeline()
{
	if(m_id) glDeleteProgram(m_id);
}
Pipeline::Pipeline(Pipeline&& other) noexcept
{
	m_id = std::exchange(other.m_id,0);
}
Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
{
	if(&other == this) return *this;
	
	m_id = std::exchange(other.m_id,0);
	return *this;
}

ComputePipeline::ComputePipeline(const Shader& computeShader,std::string_view name)
{
	m_id = glCreateProgram();
	glObjectLabel(GL_PROGRAM,m_id,name.size(),name.data());
	glAttachShader(m_id,computeShader.id());
	
	std::string log;
	if(!link(m_id,log))
	{
		glDeleteProgram(m_id);
		throw PipelineException("[LINKING FAILURE]\n",name,"\n",log);
	}
}
ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
{
	m_id =  std::exchange(other.m_id,0);
}
ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept
{
	if(&other == this) return *this;
	
	m_id = std::exchange(other.m_id,0);
	return *this;
}
ComputePipeline::~ComputePipeline()
{
	if(m_id) glDeleteProgram(m_id);
}
};
