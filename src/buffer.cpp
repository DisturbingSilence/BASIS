#include <BASIS/buffer.h>

#include <cassert>
#include <utility>

#include <glad/gl.h>

namespace BASIS
{	
Buffer::Buffer(std::size_t size,std::uint32_t flags,std::string_view name) : 
Buffer(nullptr,size,flags,name)
{
}
Buffer::Buffer(ByteSpan bytes,std::uint32_t flags,std::string_view name) : 
Buffer(bytes.data(),bytes.size_bytes(),flags,name)
{
}
Buffer::Buffer(const void* data,std::size_t size,std::uint32_t flags,std::string_view name) : m_size{size},m_flags{flags}
{
	using enum BufferFlags;
	// https://docs.gl/gl4/glBufferStorage#:~:text=are%20as%20follows%3A-,If,-flags%20contains%20GL_MAP_PERSISTENT_BIT
	assert((flags & COHERENT) ? (flags & PERSISTENT) : true);
	assert(((flags & PERSISTENT) ? (flags & READ || flags & WRITE) : true));
	glCreateBuffers(1, &m_id);
	glObjectLabel(GL_BUFFER,m_id,name.size(),name.data());
	glNamedBufferStorage(m_id, m_size, data, flags);
}

void* Buffer::map(std::uint32_t flags) noexcept
{
	m_mappedMem = glMapNamedBuffer(m_id,flags);
	return m_mappedMem;
}
void Buffer::unmap() noexcept
{
	if (m_mappedMem)
	{
		glUnmapNamedBuffer(m_id);
		m_mappedMem = nullptr;
	}
}
Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_id);
}
Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if(&other == this) return *this;
	this->~Buffer();
	return *new(this) Buffer(std::move(other));
}
Buffer::Buffer(Buffer&& other) noexcept :
m_size{other.m_size}
{
	m_id = std::exchange(other.m_id,0);
}
void Buffer::fill(std::uint32_t value,std::size_t offset,std::size_t size) noexcept
{
	const auto actualSize = size == WHOLE_BUFFER ? m_size : size;
    assert(actualSize % 4 == 0 && "Size must be a multiple of 4 bytes");
    glClearNamedBufferSubData(m_id,
                              GL_R32UI,
                              offset,
                              actualSize,
                              GL_RED_INTEGER,
                              GL_UNSIGNED_INT,
                              &value);
}
void Buffer::invalidate(std::size_t offset,std::size_t size) noexcept
{
	glInvalidateBufferSubData(m_id,offset,size == WHOLE_BUFFER ? m_size : size);
}
void Buffer::update(ByteSpan bytes,std::size_t offs) noexcept
{
	assert((m_flags & BufferFlags::DYNAMIC) && "Can't update non-dynamic buffers");
	assert(bytes.size_bytes() + offs <= m_size && "Buffer overflow");
	glNamedBufferSubData(m_id,offs,bytes.size_bytes(),bytes.data());
}


}
