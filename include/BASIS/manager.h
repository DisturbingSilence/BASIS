#pragma once

#include <BASIS/types.h>
#include <BASIS/buffer.h>

#include <string>
#include <memory>
#include <utility>
#include <cstdint>
#include <optional>
#include <functional>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

namespace BASIS
{

struct Sampler;
struct Texture;
struct Material;
struct GLTFModel;
struct SamplerInfo;

// texture/model/sampler creation/loading and caching
struct Manager
{
	Manager() = default;
	~Manager();
	
	Manager(const Manager&) = delete;
	Manager& operator=(const Manager&) = delete;
	Manager(Manager&&);
	Manager& operator=(Manager&&);

	// each `get***()` function returns view into structure owned by manager
	const Sampler* getSampler(const SamplerInfo& info) noexcept;
	
	const Texture* getTexture(std::uint64_t uniqueHash);
	const Texture* getTexture(std::uint64_t uniqueHash,std::string_view path,Format fmt = Format::UNDEFINED);
	const Texture* getTexture(std::uint64_t uniqueHash,const std::byte* px,std::size_t size,Format fmt = Format::UNDEFINED);
	const Texture* getTexture(std::uint64_t uniqueHash,const std::uint8_t* px,std::size_t size,Format fmt = Format::UNDEFINED);
	
	const GLTFModel* getModel(std::uint64_t uniqueHash);
	const GLTFModel* getModel(std::uint64_t uniqueHash,std::string_view);
	
	// for inserting hand crafted assets, will throw AssetException if asset with such hash already exists
	void insertModel(std::uint64_t uniqueHash,GLTFModel&& model);
	void insertTexture(std::uint64_t uniqueHash,Texture&& tex);
	
	// used to filter needed data from Material struct and upload it into ubo
	// (maybe you don't want all pbr bells and whistles)
	// getModel() asserts if this one is not provided
	std::function<Buffer(const std::vector<Material>&)> materialUploadCallback{};
		
	private:
	std::unordered_map<std::uint64_t,std::unique_ptr<Sampler>> m_samplers;
	std::unordered_map<std::uint64_t,std::unique_ptr<Texture>> m_textures;
	std::unordered_map<std::uint64_t,std::unique_ptr<GLTFModel>> m_models;
};
struct Primitive 
{
	std::uint32_t firstIdx{};
	std::uint32_t idxCount{};
	std::uint32_t materialIdx{};
};
struct Mesh 
{
	std::vector<Primitive> primitives;
};

enum MaterialFlags : std::uint32_t 
{
	None = 0,
	HasBaseColorTexture = 1 << 0,
	HasMetallicRoughnessTexture = 1 << 1,
	isDoubleSided = 1 << 2,
	isUnlit = 1 << 3
};
struct Node
{
	Mesh			mesh;
	std::int32_t		parent{-1};
	glm::mat4		matrix{1.f};
	glm::vec3		translation{1.f};
	glm::vec3		scale{1.f};
	glm::quat		rotation{};
	std::vector<std::size_t> children;
};

struct Vertex
{
	glm::vec3 pos{};
	glm::vec3 normal{};
	glm::vec2 uv{};
	glm::vec4 color{1.f};
};
struct GltfTexture
{
	std::uint32_t imageIdx{};
	std::uint32_t samplerIdx{};
};

struct Material
{
	glm::vec4 baseColorFactor{};
	std::uint64_t baseColorTexture{};
	std::uint64_t metallicRoughnessTexture{};
	
	float alphaMask{};
	float alphaCutoff{0.5f};
	float metallicFactor{1.f};
	float roughnessFactor{1.f};
	
	std::uint32_t flags{};
};
struct GLTFModel
{
	std::vector<Node>		nodes;
	std::vector<Material>		materials;
	std::vector<GltfTexture>	textures;
	std::vector<const Texture*>	images;
	std::vector<const Sampler*>	samplers;
	
	std::optional<Buffer>		idxBuffer;
	std::optional<Buffer>		vertexBuffer;
	std::optional<Buffer>		materialBuffer;
};

}

