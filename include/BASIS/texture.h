#pragma once

#include <BASIS/types.h>
#include <BASIS/interfaces.h>

#include <cstdint>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>

namespace BASIS
{

struct TextureUpdateInfo
{
	// info for updating compressed and non compressed textures
	std::uint32_t level{};
	glm::ivec3 offset{};
	glm::ivec3 extent{};
	// either used as raw data ptr or an offset into PBO, when copying buffer->texture
	const void* data{};

	// info for updating non compressed textures
	UploadType type = UploadType::INFER_TYPE;
	UploadFormat fmt = UploadFormat::INFER_FMT;
	
	/// size of rows in array, if 0 - tightly packed. 2D & 3D
	std::uint32_t rowLength = 0;
	/// number of rows in array,if 0 - tightly packed. 3D
	std::uint32_t imageHeight = 0;
};
struct TextureCreateInfo
{
	Format			fmt{};
	std::uint32_t	mipLevels{};
	std::uint32_t	arrayLayers{};
	glm::uvec3 		extent{};
	ImageType		type{};
	SampleCount 	samples{};
};
struct SamplerInfo
{
	bool operator==(const SamplerInfo&) const noexcept = default;
	
	using enum AddressMode;
	using enum Filter;
	
	float lodBias{0};
	float minLod{-1000};
	float maxLod{1000};
	
	bool compareEnable = false;
	Filter minFilter = LINEAR;
	Filter magFilter = LINEAR;
	Filter mipmapFilter = NONE;
	SampleCount anisotropy = SampleCount::SAMPLES_1;
	AddressMode addressModeU = CLAMP_EDGE;
	AddressMode addressModeV = CLAMP_EDGE;
	AddressMode addressModeW = CLAMP_EDGE;
	CompareMode compareMode = CompareMode::NEVER;
};
// can be created standalone, but better use getSampler in Manager class
struct Sampler : public IGLObject
{
	Sampler(const SamplerInfo& inf);
	Sampler& operator=(Sampler&&) noexcept;
	Sampler(Sampler&&) noexcept;
	const SamplerInfo& info() const noexcept { return m_info; }
	private:
	SamplerInfo m_info{};
	Sampler(std::uint32_t id,const SamplerInfo& inf);
	friend class Manager;
};
struct Texture : public IGLObject
{	
	explicit Texture(const TextureCreateInfo& info,std::string_view name="");
	
	Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    ~Texture();
	const TextureCreateInfo& info() const noexcept{ return m_info; }

	void update(const TextureUpdateInfo& info);
	void mipmap();
	
	[[nodiscard]]std::uint64_t makeBindless(const Sampler& sampler) const noexcept;
	
	std::uint64_t bindlessHandle() const noexcept { return m_bindlessHandle; }
	private:
	TextureCreateInfo m_info{};
	// user may make bindless handle from view given by getTexture()
	mutable std::uint64_t m_bindlessHandle{};
};

// non-member texture-related functions

Texture createTexture2D(glm::uvec2 size,Format fmt,std::string_view name="");
Texture createTexture2DMip(glm::ivec2 size,Format fmt,std::uint32_t mipMaps,std::string_view name="");


Texture loadTexture(std::string_view filePath,Format fmt = Format::UNDEFINED);
Texture loadTexture(const std::byte* bytes,std::size_t size,Format fmt = Format::UNDEFINED);
Texture loadTexture(const std::uint8_t* bytes,std::size_t size,Format fmt = Format::UNDEFINED);

struct Buffer;
void copyBufferToTexture(const Buffer& src,Texture& dst,const TextureUpdateInfo& inf);
void saveTexture(std::string_view p,const Texture& tex,std::int32_t level = 0,bool overwrite = true);
}
