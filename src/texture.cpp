#include <BASIS/buffer.h>
#include <BASIS/texture.h>
#include <BASIS/exception.h>

#include <utility>
#include <filesystem>

#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace BASIS
{
Texture::Texture(const TextureCreateInfo& info,std::string_view name) : m_info{info}
{
	glCreateTextures(imageTypeTo(m_info.type,BITMASK::FORMAT_GL), 1, &m_id);
	uint32_t fmt = formatTo(m_info.fmt,BITMASK::FORMAT_GL);

    switch (m_info.type)
    {
	case ImageType::TEX_1D:
		glTextureStorage1D(
		m_id, 
		m_info.mipLevels, 
		fmt, 
		m_info.extent.x);
	break;
	
	case ImageType::TEX_2D:
		glTextureStorage2D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.extent.y);
	break;
	
	case ImageType::TEX_3D:
		glTextureStorage3D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.extent.y,
		m_info.extent.z);
	break;
	
	case ImageType::TAR_1D:
		glTextureStorage2D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.arrayLayers);
	break;
	
	case ImageType::TAR_2D:
		glTextureStorage3D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.extent.y,
		m_info.arrayLayers);
	break;
	
	case ImageType::TEX_CUBEMAP:
		glTextureStorage2D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.extent.y);
	break;

	case ImageType::TAR_CUBEMAP:
		glTextureStorage3D(
		m_id,
		m_info.mipLevels,
		fmt,
		m_info.extent.x,
		m_info.extent.y,
		m_info.arrayLayers);
	break;
	
	case ImageType::TEX_MSAMPLE_2D:
		glTextureStorage2DMultisample(
		m_id,
		enumToGL(m_info.samples),
		fmt,
		m_info.extent.x,
		m_info.extent.y,
		GL_TRUE);
	break;
	
	case ImageType::TAR_MSAMPLE_2D:
		glTextureStorage3DMultisample(
		m_id,
		enumToGL(m_info.samples),
		fmt,
		m_info.extent.x,
		m_info.extent.y,
		m_info.arrayLayers,
		GL_TRUE);
	break;
    }
	glObjectLabel(GL_TEXTURE,m_id,name.size(),name.data());
}
Texture::Texture(Texture&& other) noexcept :
m_info{std::move(other.m_info)},
m_bindlessHandle{other.m_bindlessHandle}
{
	m_id = std::exchange(other.m_id,0);
}
Texture& Texture::operator=(Texture&& other) noexcept
{
	if(&other == this) return *this;
	m_info = other.m_info;
	m_bindlessHandle = std::exchange(m_bindlessHandle,0);
	m_id = std::exchange(other.m_id,0);
	return *this;
}
Sampler& Sampler::operator=(Sampler&& other) noexcept
{
	if(&other == this) return *this;
	m_info = other.m_info;
	m_id = std::exchange(other.m_id,0);
	return *this;
}

Sampler::Sampler(Sampler&& other) noexcept :
m_info{std::move(other.m_info)}
{
	m_id = std::exchange(other.m_id,0);
}
Sampler::Sampler(std::uint32_t id,const SamplerInfo& inf) : m_info{inf}
{
	m_id = id;
}
Sampler::Sampler(const SamplerInfo& inf) : m_info{inf}
{
	glCreateSamplers(1, &m_id);

    glSamplerParameteri(m_id,GL_TEXTURE_COMPARE_MODE,inf.compareEnable ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
    glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_FUNC,enumToGL(inf.compareMode));

    glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, inf.magFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);
    glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, enumToGL(inf.minFilter));

    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S, enumToGL(inf.addressModeU));
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T, enumToGL(inf.addressModeV));
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_R, enumToGL(inf.addressModeW));

    glSamplerParameterf(m_id,GL_TEXTURE_MAX_ANISOTROPY,static_cast<float>(enumToGL(inf.anisotropy)));
    glSamplerParameterf(m_id, GL_TEXTURE_LOD_BIAS, inf.lodBias);
    glSamplerParameterf(m_id, GL_TEXTURE_MIN_LOD, inf.minLod);
    glSamplerParameterf(m_id, GL_TEXTURE_MAX_LOD, inf.maxLod);
}
Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

static void updateImageImpl(uint32_t m_id,const TextureCreateInfo& m_info,const TextureUpdateInfo& info)
{	
	// dont really need to support compressed textures as of now
	//assert(!formatTo(m_info.fmt,BITMASK::IS_COMPRESSED) && "Cannot update compressed image");
	
	UploadFormat uploadFmt = static_cast<UploadFormat>(formatTo(m_info.fmt,BITMASK::UPLOAD_FORMAT));
	auto format = uploadFmtToGL(info.fmt == UploadFormat::INFER_FMT ? uploadFmt:info.fmt);
	auto type = info.type == UploadType::INFER_TYPE ? getFormatType(m_info.fmt) : enumToGL(info.type);

	
    glPixelStorei(GL_UNPACK_ROW_LENGTH, info.rowLength);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, info.imageHeight);
    switch (imageTypeTo(m_info.type,BITMASK::DIMENSIONS))
    {
    case 1:
		glTextureSubImage1D(
		m_id, 
		info.level, 
		info.offset.x, 
		info.extent.x, 
		format, 
		type, 
		info.data); 
	break;
	case 2:
		glTextureSubImage2D(
		m_id,                       
		info.level,
		info.offset.x,
		info.offset.y,
		info.extent.x,
		info.extent.y,
		format,
		type,
		info.data);
      break;
    case 3:
		glTextureSubImage3D(
		m_id,
		info.level,
		info.offset.x,
		info.offset.y,
		info.offset.z,
		info.extent.x,
		info.extent.y,
		info.extent.z,
		format,
		type,
		info.data);
      break;
    }
}
void copyBufferToTexture(const Buffer& src,Texture& dst,const TextureUpdateInfo& inf)
{
	assert(src.mappedMem() == nullptr && "Buffer must be unmapped before copying");
	glPixelStorei(GL_UNPACK_ROW_LENGTH, inf.rowLength);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, inf.imageHeight);
	
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, src.id());

    updateImageImpl(dst.id(),dst.info(),inf);
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
void Texture::update(const TextureUpdateInfo& info)
{
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	updateImageImpl(m_id,m_info,info);
}
void Texture::mipmap()
{
	glGenerateTextureMipmap(m_id);
}
Texture createTexture2D(glm::uvec2 size,Format fmt,std::string_view name)
{
	return Texture(
	TextureCreateInfo{
		.fmt = fmt,
		.mipLevels = 1,
		.arrayLayers = 1,
		.extent = {size.x,size.y,1},
		.type = ImageType::TEX_2D,	
		.samples = SampleCount::SAMPLES_1,	
	},name);
}
Texture createTexture2DMip(glm::uvec2 size,Format fmt,uint16_t mipMaps,std::string_view name)
{
	return Texture(
	TextureCreateInfo{
		.fmt = fmt,
		.mipLevels = mipMaps,
		.arrayLayers = 1,
		.extent = {size.x,size.y,1},
		.type = ImageType::TEX_2D,	
		.samples = SampleCount::SAMPLES_1,	
	},name);
}
static Texture constructSTBI(bool SRGB,const unsigned char* bytes,size_t size,std::string_view file="")
{
	int w{},h{},channels{};
	auto* px = bytes ? stbi_load_from_memory(bytes,size,&w,&h,&channels,0) : stbi_load(file.data(),&w,&h,&channels,0);
	if(!px) 
	{
		throw AssetException("STBI failed to load ",bytes ? "mem" : file," error message:",stbi_failure_reason());
	}
	Format fmt{};
	switch(channels)
	{
		case 1: fmt = Format::R8; break;
		case 2: fmt = Format::RG8; break;
		case 3: fmt = SRGB ? Format::SRGB8 : Format::RGB8; break;
		case 4: fmt = SRGB ? Format::SRGBA8 : Format::RGBA8; break;
	}
	auto tex = createTexture2D(glm::uvec2(w,h),fmt);
	tex.update(
	{
		.extent = {w,h,1},
		.type = UploadType::UBYTE,
		.data = px
	});
	stbi_image_free(px);
	return tex;
}
Texture loadTexture(std::string_view filePath,bool SRGB)
{
	if(!std::filesystem::exists(filePath)) 
	{
		throw FileException(filePath," not found");
	}
	auto format = filePath.substr(filePath.find_last_of('.')+1);
	switch(hash_64(format))
	{
		case "jpg"_hash:
		case "png"_hash:
		case "bmp"_hash:
		case "tga"_hash:
		return constructSTBI(SRGB,nullptr,0,filePath);
		default : 
		throw FileException(filePath," unsupported texture format");
		
	};	
}

Texture loadTexture(const unsigned char* bytes,size_t size,bool SRGB)
{
	return constructSTBI(SRGB,bytes,size);
}
Texture loadTexture(const std::byte* bytes,size_t size,bool SRGB)
{
	return loadTexture(reinterpret_cast<const unsigned char*>(bytes),size);
}

void saveTexture(std::string_view filePath,const Texture& tex,std::int32_t level,bool overwrite)
{
	if(std::filesystem::exists(filePath) && !overwrite)
	{
		throw FileException(filePath," already exists");
	}
	const auto info = tex.info();
	size_t bufSize = info.extent.x * info.extent.y * formatTo(info.fmt,BITMASK::SIZE_GL);
	
	unsigned char* pixels = new unsigned char[bufSize];
	
	glGetTextureImage(tex.id(),
	level <= info.mipLevels ? level : 0,
	uploadFmtToGL(static_cast<UploadFormat>(formatTo(info.fmt,BITMASK::UPLOAD_FORMAT))),
	enumToGL(UploadType::UBYTE),
	bufSize,
	pixels);
	
	auto channels = formatTo(info.fmt,BITMASK::SIZE_GL);
	switch(hash_64( filePath.substr(filePath.find_last_of('.')+1)))
	{
		case ".png"_hash:
		stbi_write_png(filePath.data(),info.extent.x,info.extent.y,channels,pixels,info.extent.x*channels);
		break;
		case ".bmp"_hash:
		stbi_write_bmp(filePath.data(),info.extent.x,info.extent.y,channels,pixels);
		break;
		case ".jpg"_hash:
		stbi_write_jpg(filePath.data(),info.extent.x,info.extent.y,channels,pixels,100);
		break;
		case ".tga"_hash:
		stbi_write_tga(filePath.data(),info.extent.x,info.extent.y,channels,pixels);
		break;
		default:
		assert("unknown file format");
	};
	delete[] pixels;
	
}
std::uint64_t Texture::makeBindless(const Sampler& sampler) const noexcept
{
    assert(m_bindlessHandle == 0 && "Bindless handle already present");
    m_bindlessHandle = glGetTextureSamplerHandleARB(m_id, sampler.id());
    assert(m_bindlessHandle != 0 && "Bindless handle creation failed");
    glMakeTextureHandleResidentARB(m_bindlessHandle);
    return m_bindlessHandle;
}

};
