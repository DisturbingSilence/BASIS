#include <BASIS/buffer.h>
#include <BASIS/texture.h>
#include <BASIS/exception.h>

#include <cstring>
#include <utility>
#include <filesystem>

#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

//#define KHRONOS_STATIC
#include <ktx.h>

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
static std::uint32_t getBlockCompressedImageSize(
	Format format, 
	std::uint32_t width, 
	std::uint32_t height, 
	std::uint32_t depth)
{
	assert(formatTo(format,BITMASK::IS_COMPRESSED));
	// BCn formats store 4x4 blocks of pixels, even if the dimensions aren't a multiple of 4
	// We round up to the nearest multiple of 4 for width and height, but not depth, since
	// 3D BCn images are just multiple 2D images stacked
	width = (width + 4 - 1) & -4;
	height = (height + 4 - 1) & -4;

	switch (format)
	{
		// BC1 and BC4 store 4x4 blocks with 64 bits (8 bytes)
		case Format::COMPRESSED_RGB_S3TC_DXT1_EXT:
		case Format::COMPRESSED_RGBA_S3TC_DXT1_EXT:
		case Format::COMPRESSED_SRGB_S3TC_DXT1_EXT:
		case Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
		case Format::COMPRESSED_RED_RGTC1:
		case Format::COMPRESSED_SIGNED_RED_RGTC1:
		return width * height * depth / 2;

		// BC3, BC5, BC6, and BC7 store 4x4 blocks with 128 bits (16 bytes)
		case Format::COMPRESSED_RGBA_S3TC_DXT3_EXT:
		case Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		case Format::COMPRESSED_RGBA_S3TC_DXT5_EXT:
		case Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		case Format::COMPRESSED_RG_RGTC2:
		case Format::COMPRESSED_SIGNED_RG_RGTC2:
		case Format::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		case Format::COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		case Format::COMPRESSED_RGBA_BPTC_UNORM:
		case Format::COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
		return width * height * depth;
		default: return {};
	}
}
static void updateCompressedImageImpl(std::uint32_t m_id,const TextureCreateInfo& m_info,const TextureUpdateInfo& info)
{
	const auto format = formatTo(m_info.fmt,BITMASK::FORMAT_GL);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);

    switch (imageTypeTo(m_info.type,BITMASK::DIMENSIONS))
    {
    case 1:
	glCompressedTextureSubImage1D(
		m_id,
		info.level,
		info.offset.x,
		info.extent.x,
		format,
		getBlockCompressedImageSize(m_info.fmt,info.extent.x,1,1),
		info.data);
    	break;
    case 2:
	glCompressedTextureSubImage2D(
		m_id,
		info.level,
		info.offset.x,
		info.offset.y,
		info.extent.x,
		info.extent.y,
		format,
		getBlockCompressedImageSize(m_info.fmt, info.extent.x, info.extent.y, 1),
		info.data);
		break;
    case 3:
	glCompressedTextureSubImage3D(
		m_id,
		info.level,
		info.offset.x,
		info.offset.y,
		info.offset.z,
		info.extent.x,
		info.extent.y,
		info.extent.z,
		format,
		getBlockCompressedImageSize(m_info.fmt, info.extent.x, info.extent.y, info.extent.z),
		info.data);
		break;
    default: assert(false);
    }
}
static void updateImageImpl(std::uint32_t m_id,const TextureCreateInfo& m_info,const TextureUpdateInfo& info)
{	
	UploadFormat uploadFmt = static_cast<UploadFormat>(formatTo(m_info.fmt,BITMASK::UPLOAD_FORMAT));
	auto format = uploadFmtToGL(info.fmt == UploadFormat::INFER_FMT ? uploadFmt : info.fmt);
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
	if(formatTo(m_info.fmt,BITMASK::IS_COMPRESSED))
	{
		updateCompressedImageImpl(m_id,m_info,info);
	}
	else
	{
		updateImageImpl(m_id,m_info,info);
	}

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
Texture createTexture2DMip(glm::uvec2 size,Format fmt,std::uint32_t mipMaps,std::string_view name)
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

static ImageType getImageType(ktxTexture2* ktx)
{
	using enum ImageType;
	if(ktx->isArray)
	{
		if(ktx->isCubemap) return TAR_CUBEMAP;
		switch(ktx->numDimensions)
		{
			case 2: return TAR_1D;
			case 3: return TAR_2D;

			// should never happen
			default: assert(false);
		};
	}
	else
	{
		if(ktx->isCubemap) return TEX_CUBEMAP;
		switch(ktx->numDimensions)
		{
			case 1: return TEX_1D;
			case 2: return TEX_2D;
			case 3: return TEX_3D;

			// should never happen
			default: assert(false);
		};
	}
	return {};
}

static Texture constructKTX(Format fmt,const std::uint8_t* bytes,std::size_t size,std::string_view file="")
{
	ktxTexture2* ktx{};
	KTX_error_code result{};
	if(bytes)
	{
		result = ktxTexture2_CreateFromMemory(bytes,size,KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,&ktx);
	}
	else
	{
		result = ktxTexture2_CreateFromNamedFile(file.data(),KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,&ktx);
	}
	if(result != KTX_SUCCESS) throw AssetException("Failed to create ktx texture[",ktxErrorString(result),']');

	if(fmt == Format::UNDEFINED)
	{
		fmt = Format::COMPRESSED_RGBA_BPTC_UNORM;
		if(ktxTexture2_NeedsTranscoding(ktx))
		{
			result = ktxTexture2_TranscodeBasis(ktx, KTX_TTF_BC7_RGBA, KTX_TF_HIGH_QUALITY);
			if(result != KTX_SUCCESS) throw AssetException("Failed to transcode ktx texture[",ktxErrorString(result),']');
		}
		else
		{
			switch (ktx->vkFormat)
			{
				case 131: fmt = Format::COMPRESSED_RGB_S3TC_DXT1_EXT;
				case 132: fmt = Format::COMPRESSED_SRGB_S3TC_DXT1_EXT;
				case 133: fmt = Format::COMPRESSED_RGBA_S3TC_DXT1_EXT;
				case 134: fmt = Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
				case 135: fmt = Format::COMPRESSED_RGBA_S3TC_DXT3_EXT;
				case 136: fmt = Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
				case 137: fmt = Format::COMPRESSED_RGBA_S3TC_DXT5_EXT;
				case 138: fmt = Format::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
				case 139: fmt = Format::COMPRESSED_RED_RGTC1;
				case 140: fmt = Format::COMPRESSED_SIGNED_RED_RGTC1;
				case 141: fmt = Format::COMPRESSED_RG_RGTC2;
				case 142: fmt = Format::COMPRESSED_SIGNED_RG_RGTC2;
				case 143: fmt = Format::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
				case 144: fmt = Format::COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
				case 145: fmt = Format::COMPRESSED_RGBA_BPTC_UNORM;
				case 146: fmt = Format::COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
				default:  fmt = Format::UNDEFINED;
			}
		}
	}
	glm::uvec3 imageExtent = {ktx->baseWidth,ktx->baseHeight,ktx->baseDepth};
	Texture output = Texture(TextureCreateInfo{
		.fmt = fmt,
		.mipLevels = ktx->numLevels,
		.arrayLayers = ktx->numLayers,
		.extent = imageExtent,
		.type = getImageType(ktx),
		.samples = SampleCount::SAMPLES_1
	});

	for (std::uint32_t level{}; level < ktx->numLevels; ++level)
	{
		std::uint32_t width = std::max(imageExtent.x >> level, 1u);
		std::uint32_t height = std::max(imageExtent.y >> level, 1u);
		std::uint32_t depth = std::max(imageExtent.z >> level, 1u);

		//ktx->numFaces is either 6(cubemap) or 1
		for(std::uint32_t face{}; face < ktx->numFaces; ++face)
		{
			std::size_t offset{};
			ktxTexture_GetImageOffset(ktxTexture(ktx), level, 0, face, &offset);
			output.update({
				.level = level,
				.offset = {0,0,face},
				.extent = {width,height,depth},
				.data = ktx->pData + offset
			});

		}
	}
	return output;
}

static Texture constructSTBI(Format fmt,const unsigned char* bytes,size_t size,std::string_view file="")
{
	int w{},h{},channels{};
	auto* px = bytes ? stbi_load_from_memory(bytes,size,&w,&h,&channels,0) : stbi_load(file.data(),&w,&h,&channels,0);
	if(!px) 
	{
		throw AssetException("STBI failed to load ",bytes ? "mem" : file," error message:",stbi_failure_reason());
	}
	if(fmt == Format::UNDEFINED)
	{
		switch(channels)
		{
			case 1: fmt = Format::R8;		break;
			case 2: fmt = Format::RG8;		break;
			case 3: fmt = Format::RGB8;		break;
			case 4: fmt = Format::RGBA8;	break;
		}
	}

	auto tex = createTexture2D(glm::uvec2(w,h),fmt);
	tex.update({
		.extent = {w,h,1},
		.data = px,
		.type = UploadType::UBYTE,
	});
	stbi_image_free(px);
	return tex;
}
Texture loadTexture(std::string_view filePath,Format fmt)
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
			return constructSTBI(fmt,nullptr,0,filePath);

		case "ktx"_hash: 
			throw AssetException("KTX is outdated, please upgrade to KTX2");

		case "ktx2"_hash:
			return constructKTX(fmt,nullptr,0,filePath);

		default : 
		throw FileException(filePath," unsupported texture format");
		
	};	
}

Texture loadTexture(const std::uint8_t* bytes,std::size_t size,Format fmt)
{
	static constexpr std::uint8_t ktxMagic[12] ={0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
	if(std::memcmp(ktxMagic,bytes,12) == 0)
	{
		return constructKTX(fmt,bytes,size);
	}
	return constructSTBI(fmt,bytes,size);
}
Texture loadTexture(const std::byte* bytes,std::size_t size,Format fmt)
{
	return loadTexture(reinterpret_cast<const std::uint8_t*>(bytes),size,fmt);
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
	using namespace literals;
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
