#pragma once

#include <BASIS/interfaces.h>

#include <span>
#include <string_view>
	
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

struct ByteSpan : public std::span<const std::byte>
{
	template<TriviallyCopyable T>
	ByteSpan(const T& bytes) : 
	std::span<const std::byte>(std::as_bytes(std::span{&bytes, static_cast<std::size_t>(1)}))
	{} 
	template<TriviallyCopyable T>
	ByteSpan(std::span<T> bytes) : std::span<const std::byte>(std::as_bytes(bytes))
	{} 
	template<TriviallyCopyable T>
	ByteSpan(std::span<const T> bytes) : std::span<const std::byte>(std::as_bytes(bytes))
	{} 
	
};

namespace BASIS
{
enum class AccessFlags : std::uint32_t;
constexpr inline std::uint64_t WHOLE_BUFFER = static_cast<uint64_t>(-1);

struct Buffer : public IGLObject
{
	Buffer& operator=(Buffer&&) noexcept;
	Buffer(Buffer&&) noexcept;
	
	explicit Buffer(ByteSpan data,std::uint32_t flags,std::string_view name="");
	explicit Buffer(std::size_t size,std::uint32_t flags,std::string_view name="");
	
	void update(ByteSpan,std::size_t offs = 0) noexcept;
	
	void* map(AccessFlags flags) noexcept;
	void unmap() noexcept;
	
	void fill(std::uint32_t value,std::size_t offset = 0,std::size_t size = WHOLE_BUFFER) noexcept;
	void invalidate(std::size_t offset = 0,std::size_t size = WHOLE_BUFFER) noexcept;
	~Buffer();
	size_t size() const noexcept { return m_size; }
	void* mappedMem() const noexcept { return m_mappedMem; }
	
	protected:
	
	explicit Buffer(const void* data,std::size_t size,std::uint32_t flags,std::string_view name="");
	
	std::size_t m_size{};
	std::uint32_t m_flags{};
	void* m_mappedMem{nullptr};
};
template<TriviallyCopyable T>
struct TypedBuffer : public Buffer
{
	explicit TypedBuffer(uint32_t flags,std::string_view name="") : 
	Buffer(sizeof(T),flags,name) {}
	explicit TypedBuffer(size_t amount,std::uint32_t flags,std::string_view name="") : 
	Buffer(sizeof(T)*amount,flags,name) {}
	explicit TypedBuffer(const T& data,std::uint32_t flags,std::string_view name="") : 
	Buffer(&data,sizeof(T),flags,name) {}
	explicit TypedBuffer(const std::span<const T> data,std::uint32_t flags,std::string_view name="") : 
	Buffer(data,flags,name) {}
	TypedBuffer(TypedBuffer&& other) noexcept = default;
	TypedBuffer& operator=(TypedBuffer&& other) noexcept = default;
	void update(const T& data, std::size_t offs=0)
	{
		Buffer::update(data, sizeof(T) * offs);
	}
	void update(std::span<const T> data, std::size_t offs=0)
	{
		Buffer::update(data, sizeof(T) * offs);
	}
};
};
