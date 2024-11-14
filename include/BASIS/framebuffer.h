#pragma once

#include <BASIS/interfaces.h>
#include <BASIS/texture.h>

#include <vector>
#include <optional>
#include <string_view>
namespace BASIS
{

// expand this to optionally use renderbuffers
struct FramebufferCreateInfo
{
	std::vector<Texture> colorAttachments;
	std::optional<Texture> depthAttachment;
	std::optional<Texture> stencilAttachment;
	bool separateDepthStencil{true}; // if true depth and stencil are separate, if false both are rendered onto depth attachment
};

struct Framebuffer : public IGLObject
{
	explicit Framebuffer(FramebufferCreateInfo&& info,std::string_view name="");
	~Framebuffer();

	Framebuffer& operator=(Framebuffer&&) noexcept;
	Framebuffer(Framebuffer&&) noexcept;
	
	const FramebufferCreateInfo& info() const noexcept { return m_info; }
	private:
	FramebufferCreateInfo m_info;
};
};