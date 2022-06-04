#include "VertexBuffer.hpp"

#include <glad/glad.h>

namespace dgame {

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VertexBuffer::VertexBuffer(uint32_t size)
{
   glCreateBuffers(1, &m_RendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
   glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(float* vertices, uint32_t size)
{
   glCreateBuffers(1, &m_RendererID);
   glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
   glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
   glDeleteBuffers(1, &m_RendererID);
}

void
VertexBuffer::Bind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void
VertexBuffer::Unbind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
VertexBuffer::SetData(const void* data, uint32_t size) const
{
   glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
   glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count) : m_Count(count)
{
   glCreateBuffers(1, &m_RendererID);

   // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
   // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state.
   glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
   glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
   glDeleteBuffers(1, &m_RendererID);
}

void
IndexBuffer::Bind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void
IndexBuffer::Unbind() const
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace dgame
