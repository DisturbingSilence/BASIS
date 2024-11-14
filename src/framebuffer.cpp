#include <BASIS/framebuffer.h>

#include <cassert>
#include <numeric>
#include <utility>

#include <glad/gl.h>
namespace BASIS
{
Framebuffer::Framebuffer(FramebufferCreateInfo&& info,std::string_view name) : m_info{std::forward<FramebufferCreateInfo>(info)}
{
	glCreateFramebuffers(1, &m_id);
	for(std::uint32_t i{};i < m_info.colorAttachments.size();i++)
	{
		glNamedFramebufferTexture(m_id, static_cast<std::uint32_t>(GL_COLOR_ATTACHMENT0 + i), m_info.colorAttachments[i].id(), 0);
	}
  std::vector<std::uint32_t> drawBuffers(m_info.colorAttachments.size());
  std::iota(drawBuffers.begin(),drawBuffers.end(),GL_COLOR_ATTACHMENT0);
  glNamedFramebufferDrawBuffers(m_id, static_cast<std::uint32_t>(drawBuffers.size()), drawBuffers.data());
  if (m_info.separateDepthStencil)
  {
    if (m_info.depthAttachment)
    {
      glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, m_info.depthAttachment->id(), 0);
    }

    if (m_info.stencilAttachment)
    {
      glNamedFramebufferTexture(m_id, GL_STENCIL_ATTACHMENT, m_info.stencilAttachment->id(), 0);
    }
  }
  else
  {
    assert(m_info.depthAttachment && m_info.stencilAttachment);

    glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, m_info.depthAttachment->id(), 0);
    glNamedFramebufferTexture(m_id, GL_STENCIL_ATTACHMENT, m_info.depthAttachment->id(), 0);
    
  }
}
Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &m_id);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
  if(&other == this) return *this;
  this->~Framebuffer();
  return *new(this) Framebuffer(std::move(other));
}
Framebuffer::Framebuffer(Framebuffer&& other) noexcept
{
  m_id = std::exchange(other.m_id,0);
  m_info = std::exchange(other.m_info,{});
}
}