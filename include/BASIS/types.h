// contains enums + some miscellaneous funcs
#pragma once

#include <BASIS/declare_flags.h>

#include <vector>
#include <cstdint>
#include <concepts>
#include <stdexcept>
template<typename T, typename... Ts>
concept is_any_same = (std::same_as<T, Ts> || ...);

namespace BASIS
{
template<typename T>
struct hash;

template<class T>
void hash_combine(std::size_t& seed, const T& v)
{
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<class Tuple, std::size_t Index = std::tuple_size<Tuple>::value - 1>
struct HashValueImpl
{
static void apply(std::size_t& seed, const Tuple& tuple)
{
	HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
	hash_combine(seed, std::get<Index>(tuple));
}
};

template<class Tuple>
struct HashValueImpl<Tuple, 0>
{
	static void apply(std::size_t& seed, const Tuple& tuple)
	{
		hash_combine(seed, std::get<0>(tuple));
	}
};

template<typename... TT>
struct hash<std::tuple<TT...>>
{
	size_t operator()(const std::tuple<TT...>& tt) const
	{
		std::size_t seed = 0;
		HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
		return seed;
	}
};
inline const std::uint64_t hash_64(std::string_view str) 
{
	std::uint64_t hash = 0xcbf29ce484222325;

	for(size_t i = 0; i < str.size(); ++i) {
		hash = hash ^ str[i];
		hash *= 0x100000001b3;
	}
	return hash;
}

consteval std::uint64_t hash_64_const(const char* const str, const std::uint64_t value = 0xcbf29ce484222325) noexcept 
{
	if(str[0] == '\0')
	{
		return value;
	}
	else
	{
		return hash_64_const(&str[1], (value ^ std::uint64_t((std::uint8_t)str[0])) * 0x100000001b3);
	}
}

consteval std::uint64_t operator""_hash(const char* str,std::size_t)
{
	 return hash_64_const(str);
}
enum class FloatUniform
{
	f1,
	f2,
	f3,
	f4,
	mat2,
	mat3,
	mat4,
	mat2x3,
	mat3x2,
	mat2x4,
	mat4x2,
	mat3x4,
	mat4x3,
};

enum class Cap : std::uint32_t
{
	BLEND = 0x0BE2,
	DITHER = 0x0BD0,
	CULL_FACE = 0x0B44,
	DEPTH_TEST = 0x0B71,
	LINE_SMOOTH = 0x0B20,
	MULTISAMPLE = 0x809D,
	SAMPLE_MASK = 0x8E51,
	DEPTH_CLAMP = 0x864F,
	DEBUG_OUTPUT = 0x92E0,
	SCISSOR_TEST = 0x0C11,
	STENCIL_TEST = 0x0B90,
	SAMPLE_SHADING = 0x8C36,
	COLOR_LOGIC_OP = 0x0BF2,
	POLYGON_SMOOTH = 0x0B41,
	SAMPLE_COVERAGE = 0x80A0,
	FRAMEBUFFER_SRGB = 0x8DB9,
	PRIMITIVE_RESTART = 0x8F9D,
	PROGRAM_POINT_SIZE = 0x8642,
	RASTERIZER_DISCARD = 0x8C89,
	SAMPLE_ALPHA_TO_ONE = 0x809F,
	POLYGON_OFFSET_FILL = 0x8037,
	POLYGON_OFFSET_LINE = 0x2A02,
	POLYGON_OFFSET_POINT = 0x2A01,
	SAMPLE_ALPHA_TO_COVERAGE = 0x809E,
	DEBUG_OUTPUT_SYNCHRONOUS = 0x8242,
	TEXTURE_CUBE_MAP_SEAMLESS = 0x884F,
	PRIMITIVE_RESTART_FIXED_INDEX = 0x8D69,
};

// too much convenient conversions to declare this with BASIS_DECLARE_FLAG_TYPE
enum BufferFlags : std::uint32_t
{
	READ = 0x0001,
	WRITE = 0x0002,
	CLIENT = 0x0200,
	DYNAMIC = 0x0100,
	COHERENT = 0x00000080,
	PERSISTENT = 0x00000040,
	UNSYNCHRONIZED = 0x0020,
	FLUSH_EXPLICIT = 0x0010,
	INVALIDATE_RANGE = 0x0004,
	INVALIDATE_BUFFER = 0x0008,
};
enum class AccessFlags : std::uint32_t
{
	READ_ONLY = 0x88B8,
	WRITE_ONLY = 0x88B9,
	READ_WRITE = 0x88BA,
};

enum class MaskFlagBit : std::uint32_t
{
	DEPTH	= 0x00000100,
	COLOR	= 0x00004000,
	STENCIL	= 0x00000400
};
BASIS_DECLARE_FLAG_TYPE(MaskFlags,MaskFlagBit,std::uint32_t);

enum class CullMode : std::uint32_t
{
	NONE = 0,
	BACK = 0x0405,
	FRONT = 0x0404,
	FRONT_AND_BACK = 0x0408
};
enum class FrontFace : std::uint32_t
{
	CW = 0x0900,
	CCW = 0x0901
};
enum class UploadFormat : std::uint32_t
{
	UNDEFINED = 0,
	RG,
	RED,
	RGB,
	BGR, 
	RGBA,
	BGRA,
	RG_INTEGER,
	RED_INTEGER,
	RGB_INTEGER,
	BGR_INTEGER,
	RGBA_INTEGER,
	BGRA_INTEGER,
	STENCIL_INDEX, 
	DEPTH_STENCIL,
	DEPTH_COMPONENT,
	INFER_FMT = 15,
};

constexpr std::uint32_t uploadFmtToGL(UploadFormat F)
{
	using enum UploadFormat;
	switch(F)
	{
		case RG:              return 0x8227;
		case RED:             return 0x1903;
		case RGB:             return 0x1907;
		case BGR:             return 0x80E0;
		case RGBA:            return 0x1908;
		case BGRA:            return 0x80E1;
		case RG_INTEGER:      return 0x8228;
		case RED_INTEGER:     return 0x8D94;
		case RGB_INTEGER:     return 0x8D98;
		case BGR_INTEGER:     return 0x8D9A;
		case RGBA_INTEGER:    return 0x8D99;
		case BGRA_INTEGER:    return 0x8D9B;
		case STENCIL_INDEX:   return 0x1901;
		case DEPTH_STENCIL:   return 0x84F9;
		case DEPTH_COMPONENT: return 0x1902;
		default: return 0;
	}
}
static consteval std::uint32_t imgTypeCtor(int dimensions, int glType)
{
	return dimensions << 16 | glType;
}

static consteval std::uint32_t fmtCtor(
bool isNormalized, 
std::int32_t size, 
BASIS::UploadFormat upFmt, 
bool isCompressed, 
std::uint16_t glFormat)
{
	return isNormalized << 24 | size << 21 | (std::uint32_t)upFmt << 17 | isCompressed << 16 | glFormat;
}

enum class ImageType : std::uint32_t
{
	TEX_1D = imgTypeCtor(1, 0x0DE0),
	TEX_2D = imgTypeCtor(2, 0x0DE1),
	TEX_3D = imgTypeCtor(3, 0x806F),
	TAR_1D = imgTypeCtor(2, 0x8C18),
	TAR_2D = imgTypeCtor(3, 0x8C1A),
	TEX_CUBEMAP = imgTypeCtor(3, 0x8513),
	TAR_CUBEMAP = imgTypeCtor(3, 0x9009),
	TEX_MSAMPLE_2D = imgTypeCtor(2, 0x9100),
	TAR_MSAMPLE_2D = imgTypeCtor(3, 0x9102),
};

enum class Filter : std::uint32_t
{
	NONE = 0,
	NEAREST = 0x2600,
	LINEAR = 0x2601,
	LINEAR_MIP_LINEAR = 0x2703,
	NEAREST_MIP_LINEAR = 0x2702,
	LINEAR_MIP_NEAREST = 0x2701,
	NEAREST_MIP_NEAREST = 0x2700,
};

enum class AddressMode : std::uint32_t
{
	REPEAT = 0x2901,
	CLAMP_EDGE = 0x812F,
	BORDER_CLAMP = 0x812D,
	REPEAT_MIRRORED = 0x8370,
	CLAMP_EDGE_MIRRORED = 0x8743,
};

enum class CompareMode : std::uint32_t
{
	LESS = 0x0201,
	EQUAL = 0x0202,
	NEVER = 0x0200,
	NEQUAL = 0x0205,
	LEQUAL = 0x0203,
	GEQUAL = 0x0206,
	ALWAYS = 0x0207,
	GREATER = 0x0204,
};

enum class PrimitiveMode : std::uint32_t
{
	POINTS = 0x0000,
	PATCHES = 0x000E, // tessellation
	LINES = 0x0001,
	LINE_LOOP = 0x0002,
	LINE_STRIP = 0x0003,
	TRIANGLES = 0x0004,
	TRIANGLE_FAN = 0x0006,
	TRIANGLE_STRIP = 0x0005,
};
enum class PolygonMode : std::uint32_t
{
	FILL = 0x1B02,
	LINE = 0x1B01,
	POINT = 0x1B00
};
enum class ComponentSwizzle : std::uint32_t
{
	R = 0x1903,
	G = 0x1904,
	B = 0x1905,
	A = 0x1906,
	ONE = 0x0001,
	ZERO = 0x0000,
};

enum class Factor : std::uint32_t
{
	ONE = 0x0001,
	ZERO = 0x0000,
	SRC_COLOR = 0x0300,
	DST_COLOR = 0x0306,
	SRC_ALPHA = 0x0302,
	DST_ALPHA = 0x0304,
	CONSTANT_COLOR = 0x8001,
	CONSTANT_ALPHA = 0x8003,
	SRC_ALPHA_SATURATE	= 0x0308,
	ONE_MINUS_SRC_COLOR = 0x0301,
	ONE_MINUS_DST_COLOR = 0x0307,
	ONE_MINUS_SRC_ALPHA = 0x0303,
	ONE_MINUS_DST_ALPHA = 0x0305,
	ONE_MINUS_CONSTANT_COLOR = 0x8002,
	ONE_MINUS_CONSTANT_ALPHA = 0x8004,
};
enum class ShaderType : std::uint32_t 
{
	VERTEX          = 0x8B31, 
	COMPUTE         = 0x91B9, 
	FRAGMENT        = 0x8B30,
	TESS_EVAL       = 0x8E87, 
	TESS_CONTROL    = 0x8E88, 
};

enum class BITMASK : std::uint32_t
{
	//ImageType & Format
	FORMAT_GL,   // OpenGL format

	// ImageType only
	DIMENSIONS,  // Image dimensions

	// Format only
	SIZE_GL,     // Size for vertex attributes
	TYPE_GL,
	IS_COMPRESSED,  // Check if format is compressed
	IS_NORMALIZED,  // Check if format is normalized
	UPLOAD_FORMAT,  // Used when uploading image into OpenGL
};
enum class Format : std::uint32_t 
{
	UNDEFINED = 0,
	R8 = fmtCtor(true, 1, UploadFormat::RED, false, 0x8229),  
	R16 = fmtCtor(true, 1, UploadFormat::RED, false, 0x822A),  
	RG8 = fmtCtor(true, 2, UploadFormat::RG, false, 0x822B),  
	R8I = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8231),  
	RG16 = fmtCtor(true, 2, UploadFormat::RG, false, 0x822C),  
	RGB4 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x804F),  
	RGB5 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8050),  
	RGB8 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8051),  
	R16F = fmtCtor(false, 1, UploadFormat::RED, false, 0x822D),  
	R32F = fmtCtor(false, 1, UploadFormat::RED, false, 0x822E),  
	R8UI = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8232),  
	R16I = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8233),  
	R32I = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8235), 
	RG8I = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x8237),  
	RGB10 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8052),  
	RGB12 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8053),  
	RGBA2 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8055),  
	RGBA4 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8056), 
	RGBA8 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8058),  
	SRGB8 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8C41), 
	RG16F = fmtCtor(false, 2, UploadFormat::RG, false, 0x822F), 
	RG32F = fmtCtor(false, 2, UploadFormat::RG, false, 0x8230),  
	R16UI = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8234),  
	R32UI = fmtCtor(false, 1, UploadFormat::RED_INTEGER, false, 0x8236), 
	RG8UI = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x8238),  
	RG16I = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x8239),  
	RG32I = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x823B),  
	RGB8I = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D8F),  
	RGBA12 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x805A),  
	RGBA16 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x805B), 
	RGB16F = fmtCtor(false, 3, UploadFormat::RGB, false, 0x881B),  
	RGB32F = fmtCtor(false, 3, UploadFormat::RGB, false, 0x8815), 
	RG16UI = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x823A), 
	RG32UI = fmtCtor(false, 2, UploadFormat::RG_INTEGER, false, 0x823C),  
	RGB8UI = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D7D),  
	RGB16I = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D89), 
	RGB32I = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D83), 
	RGBA8I = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D8E),  
	RGB9_E5 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x8C3D),  
	RGB5_A1 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8057), 
	RGBA16F = fmtCtor(false, 4, UploadFormat::RGBA, false, 0x881A),  
	RGBA32F = fmtCtor(false, 4, UploadFormat::RGBA, false, 0x8814),  
	RGB16UI = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D77),  
	RGB32UI = fmtCtor(false, 3, UploadFormat::RGB_INTEGER, false, 0x8D71),  
	RGBA8UI = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D7C), 
	RGBA16I = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D88),  
	RGBA32I = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D82),  
	R8_SNORM = fmtCtor(false, 1, UploadFormat::RED, false, 0x8F94),  
	R3_G3_B2 = fmtCtor(true, 3, UploadFormat::RGB, false, 0x2A10),  
	RGB10_A2 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8059),
	RGBA16UI = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D76),
	RGBA32UI = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x8D70),  
	R16_SNORM = fmtCtor(false, 1, UploadFormat::RED, false, 0x8F98), 
	RG8_SNORM = fmtCtor(false, 2, UploadFormat::RG, false, 0x8F95), 
	RGB8_SNORM = fmtCtor(false, 3, UploadFormat::RGB, false, 0x8F96),  
	RGB10_A2UI = fmtCtor(false, 4, UploadFormat::RGBA_INTEGER, false, 0x906F), 
	RGB16_SNORM = fmtCtor(false, 3, UploadFormat::RGB, false, 0x8F9A),  
	SRGBA8 = fmtCtor(true, 4, UploadFormat::RGBA, false, 0x8C43),  
	RG16_SNORM = fmtCtor(false, 2, UploadFormat::RG, false, 0x8F99),  
	RGBA8_SNORM = fmtCtor(false, 4, UploadFormat::RGBA, false, 0x8F97),
	RGBA16_SNORM = fmtCtor(false, 4, UploadFormat::RGBA, false, 0x8F9B), 
	DEPTH_COMPONENT16 = fmtCtor(true, 1, UploadFormat::DEPTH_COMPONENT, false, 0x81A5),  
	DEPTH_COMPONENT24 = fmtCtor(true, 1, UploadFormat::DEPTH_COMPONENT, false, 0x81A6),
	DEPTH_COMPONENT32 = fmtCtor(true, 1, UploadFormat::DEPTH_COMPONENT, false, 0x81A7),  
	DEPTH_COMPONENT32F = fmtCtor(true, 1, UploadFormat::DEPTH_COMPONENT, false, 0x8CAC),
	DEPTH32F_STENCIL8 = fmtCtor(true,1,UploadFormat::DEPTH_STENCIL,false,0x8CAD),
	DEPTH24_STENCIL8 = fmtCtor(true,1,UploadFormat::DEPTH_STENCIL,false,0x88F0),
	COMPRESSED_RG_RGTC2 = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8DBD),
	COMPRESSED_RED_RGTC1 = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8DBB),
	COMPRESSED_SIGNED_RG_RGTC2 = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8DBE),
	COMPRESSED_RGBA_BPTC_UNORM = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8E8C),
	COMPRESSED_SIGNED_RED_RGTC1 = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8DBC),
	COMPRESSED_RGB_S3TC_DXT1_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x83F0),
	COMPRESSED_RGBA_S3TC_DXT1_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x83F1),
	COMPRESSED_SRGB_S3TC_DXT1_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8C4C),
	COMPRESSED_RGBA_S3TC_DXT3_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x83F2),
	COMPRESSED_RGBA_S3TC_DXT5_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x83F3),
	COMPRESSED_RGB_BPTC_SIGNED_FLOAT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8E8E),
	COMPRESSED_SRGB_ALPHA_BPTC_UNORM = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8E8D),
	COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8E8F),
	COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8C4D),
	COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8C4E),
	COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = fmtCtor(false, 0, UploadFormat::UNDEFINED, true, 0x8C4F)
};
enum class IndexType : std::uint32_t
{
	UINT   = 0x1405, 
	UBYTE  = 0x1401,
	USHORT = 0x1403, 
};
constexpr std::uint32_t getFormatType(Format F)
{
	using enum Format;
	switch(F)
	{
	case R8:
	case RG8:
	case R8UI:
	case RGB8:
	case RGBA8:
	case SRGB8:
	case RG8UI:
	case RGB8UI:
	case RGBA8UI:
	case SRGBA8:
		return 0x1401;
	case R8I:
	case RG8I:
	case RGB8I:
	case RGBA8I:
	case R8_SNORM:
	case RG8_SNORM:
	case RGB8_SNORM:
	case RGBA8_SNORM:
		return 0x1400; 
	case R16UI:
	case RG16UI:
	case RGB16UI:
	case RGBA16UI:
		return 0x1403; 
	case R16F:
	case RG16F:
	case RGB16F:
	case RGBA16F:
		return 0x140B; 
	case R16I:
	case RG16I:
	case RGB16I:
	case RGBA16I:
		return 0x1402;
	case R32I:
	case RG32I:
	case RGB32I:
	case RGBA32I:
		return 0x1404; 
	case R32UI:
	case RG32UI:
	case RGB32UI:
	case RGBA32UI:
		return 0x1405; 
	case R32F:
	case RG32F:
	case RGB32F:
	case RGBA32F:
		return 0x1406; 
	default:
		return 0;
	}
}
enum class SampleCount : std::uint32_t
{
	SAMPLES_1  = 1,
	SAMPLES_2  = 2,
	SAMPLES_4  = 4,
	SAMPLES_8  = 8,
	SAMPLES_16 = 16,
	SAMPLES_32 = 32
};
enum class UploadType : std::uint32_t
{
	UNDEFINED = 0,

	INT    = 0x1404, 
	UINT   = 0x1405, 
	BYTE   = 0x1400, 
	UBYTE  = 0x1401, 
	FLOAT  = 0x1406, 
	SHORT  = 0x1402, 
	USHORT = 0x1403, 

	UBYTE_3_3_2 		= 0x8032,
	USHORT_5_6_5 		= 0x8363, 
	UINT_8_8_8_8  		= 0x8035, 
	UBYTE_2_3_3_R		= 0x8362, 
	USHORT_4_4_4_4 		= 0x8033, 
	USHORT_5_5_5_1 		= 0x8034, 
	USHORT_5_6_5_R		= 0x8364, 
	UINT_8_8_8_8_R		= 0x8367, 
	UINT_10_10_10_2 	= 0x8036,
	USHORT_4_4_4_4_R	= 0x8365, 
	USHORT_1_5_5_5_R	= 0x8366, 
	UINT_2_10_10_10_R	= 0x8368, 
	
	/// @brief For CopyTextureToBuffer and CopyBufferToTexture
	INFER_TYPE,

};
constexpr std::uint32_t formatTo(Format F,BITMASK M)
{
	std::uint32_t  val = static_cast<std::uint32_t>(F);
	using enum BITMASK;
	switch(M)
	{
		case FORMAT_GL:  return val & 0xFFFF;
		
		case SIZE_GL:		return ( val & 0xE00000  ) >> 21; 
		case IS_COMPRESSED: return ( val & 0x10000   ) >> 16;
		case IS_NORMALIZED: return ( val & 0x1000000 ) >> 24;
		case UPLOAD_FORMAT: return ( val & 0x1E0000  ) >> 17;
		
		case TYPE_GL: return getFormatType(F);
	
		default: throw std::runtime_error("Format does not contain such info");
		return 0;
	}
}
constexpr std::uint32_t imageTypeTo(ImageType T,BITMASK M)
{
	std::uint32_t val = static_cast<std::uint32_t>(T);
	using enum BITMASK;
	switch(M)
	{
		case DIMENSIONS: return (val & 0x30000) >> 16; 
		case FORMAT_GL:  return val & 0xFFFF;		 
		default: throw std::runtime_error("ImageType does not contain such info");
		return 0;
	};
}
template<typename T>
requires is_any_same<T,
					ComponentSwizzle,
					PrimitiveMode,
					FrontFace,
					CullMode,
					PolygonMode,
					SampleCount,
					AddressMode,
					CompareMode,
					ShaderType,
					UploadType,
					IndexType,
					Filter,
					Factor,
					Cap>
std::uint32_t enumToGL(T e)
{
	return static_cast<std::uint32_t >(e);
}
}
