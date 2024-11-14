#pragma once

#include <BASIS/types.h>
#include <BASIS/pipeline.h>

#include <string>

namespace BASIS
{
struct DeviceLimits
{
	std::int32_t maxTextureSize;     // GL_MAX_TEXTURE_SIZE
	std::int32_t maxTextureSize3D;   // GL_MAX_3D_TEXTURE_SIZE
	std::int32_t maxTextureSizeCube; // GL_MAX_CUBE_MAP_TEXTURE_SIZE

	float maxSamplerLodBias;       // GL_MAX_TEXTURE_LOD_BIAS
	float maxSamplerAnisotropy;    // GL_MAX_TEXTURE_MAX_ANISOTROPY
	std::int32_t maxArrayTextureLayers; // GL_MAX_ARRAY_TEXTURE_LAYERS
	std::int32_t maxViewportDims[2];    // GL_MAX_VIEWPORT_DIMS
	std::int32_t subpixelBits;          // GL_SUBPIXEL_BITS

	std::int32_t maxFramebufferWidth;  // GL_MAX_FRAMEBUFFER_WIDTH
	std::int32_t maxFramebufferHeight; // GL_MAX_FRAMEBUFFER_HEIGHT
	std::int32_t maxFramebufferLayers; // GL_MAX_FRAMEBUFFER_LAYERS
	std::int32_t maxFramebufferSamples; // GL_MAX_FRAMEBUFFER_SAMPLES
	std::int32_t maxColorAttachments;  // GL_MAX_COLOR_ATTACHMENTS
	std::int32_t maxSamples;              // GL_MAX_SAMPLES
	std::int32_t maxSamplesNoAttachments; // GL_MAX_FRAMEBUFFER_SAMPLES

	float interpolationOffsetRange[2]; // GL_MIN_FRAGMENT_INTERPOLATION_OFFSET & GL_MAX_FRAGMENT_INTERPOLATION_OFFSET
	float pointSizeGranularity;        // GL_POINT_SIZE_GRANULARITY
	float pointSizeRange[2];           // GL_POINT_SIZE_RANGE
	float lineWidthRange[2];           // GL_ALIASED_LINE_WIDTH_RANGE

	std::int32_t maxElementIndex;               // GL_MAX_ELEMENT_INDEX
	std::int32_t maxVertexAttribs;              // GL_MAX_VERTEX_ATTRIBS
	std::int32_t maxVertexAttribBindings;       // GL_MAX_VERTEX_ATTRIB_BINDINGS
	std::int32_t maxVertexAttribStride;         // GL_MAX_VERTEX_ATTRIB_STRIDE
	std::int32_t maxVertexAttribRelativeOffset; // GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET
	std::int32_t maxVertexOutputComponents;     // GL_MAX_VERTEX_OUTPUT_COMPONENTS
	std::int32_t maxTessellationControlPerVertexInputComponents; // GL_MAX_TESS_CONTROL_INPUT_COMPONENTS
	std::int32_t maxTessellationControlPerVertexOutputComponents;// GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS
	std::int32_t maxTessellationControlPerPatchOutputComponents; // GL_MAX_TESS_PATCH_COMPONENTS
	std::int32_t maxTessellationControlTotalOutputComponents;    // GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS
	std::int32_t maxTessellationEvaluationInputComponents;       // GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS
	std::int32_t maxTessellationEvaluationOutputComponents;      // GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS
	std::int32_t maxFragmentInputComponents;    // GL_MAX_FRAGMENT_INPUT_COMPONENTS
	std::int32_t texelOffsetRange[2];           // GL_MIN_PROGRAM_TEXEL_OFFSET & GL_MAX_PROGRAM_TEXEL_OFFSET
	std::int32_t textureGatherOffsetRange[2];   // GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET & GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET

	std::int32_t maxTessellationGenerationLevel; // GL_MAX_TESS_GEN_LEVEL
	std::int32_t maxPatchSize;                   // GL_MAX_PATCH_VERTICES

	std::int32_t maxUniformBufferBindings;     // GL_MAX_UNIFORM_BUFFER_BINDINGS
	std::int32_t maxUniformBlockSize;          // GL_MAX_UNIFORM_BLOCK_SIZE
	std::int32_t uniformBufferOffsetAlignment; // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
	std::int32_t maxCombinedUniformBlocks;     // GL_MAX_COMBINED_UNIFORM_BLOCKS

	std::int32_t maxShaderStorageBufferBindings;     // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
	std::int32_t maxShaderStorageBlockSize;          // GL_MAX_SHADER_STORAGE_BLOCK_SIZE
	std::int32_t shaderStorageBufferOffsetAlignment; // GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
	std::int32_t maxCombinedShaderStorageBlocks;     // GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS

	std::int32_t maxCombinedShaderOutputResources; // GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES
	std::int32_t maxCombinedTextureImageUnits;     // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS

	std::int32_t maxComputeSharedMemorySize;     // GL_MAX_COMPUTE_SHARED_MEMORY_SIZE
	std::int32_t maxComputeWorkGroupInvocations; // GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS
	std::int32_t maxComputeWorkGroupCount[3];    // GL_MAX_COMPUTE_WORK_GROUP_COUNT
	std::int32_t maxComputeWorkGroupSize[3];     // GL_MAX_COMPUTE_WORK_GROUP_SIZE

	std::int32_t maxImageUnits;                      // GL_MAX_IMAGE_UNITS
	std::int32_t maxFragmentCombinedOutputResources; // GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS
	std::int32_t maxCombinedImageUniforms;           // GL_MAX_COMBINED_IMAGE_UNIFORMS
	std::int32_t maxServerWaitTimeout;               // GL_MAX_SERVER_WAIT_TIMEOUT
};
struct DeviceProperties
{
	std::string_view vendor;
	std::string_view renderer;
	std::string_view version;
	std::string_view shadingLanguageVersion;
	std::int32_t glVersionMajor;
	std::int32_t glVersionMinor;
	DeviceLimits limits;
};

struct RenderingContext
{
	RenderingContext();
	
	DeviceProperties properties;
	
	bool isRendering{false};
	bool isComputeActive{false};
	bool isIdxBufferBound{false};
	bool lastPipelineWasCompute{false};
	
	std::uint32_t fbo{};
	std::uint32_t vao{};

	std::shared_ptr<const PipelineInfo> lastPipelineInfo;
	std::uint32_t lastBoundPipeline{};
	
	IndexType idxType{IndexType::UINT};
	PrimitiveMode primitiveMode{PrimitiveMode::TRIANGLES};
	
	bool checkExtensionSupport(std::string_view requestedExt);
};
	
};
