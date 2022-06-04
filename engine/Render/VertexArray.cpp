#include "VertexArray.hpp"

#include <glad/glad.h>

namespace dgame {

static GLenum
ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
   switch (type)
   {
      case ShaderDataType::Float:
      case ShaderDataType::Float2:
      case ShaderDataType::Float3:
      case ShaderDataType::Float4:
      case ShaderDataType::Mat3:
      case ShaderDataType::Mat4:
         return GL_FLOAT;
      case ShaderDataType::Int:
      case ShaderDataType::Int2:
      case ShaderDataType::Int3:
      case ShaderDataType::Int4:
         return GL_INT;
      case ShaderDataType::Bool:
         return GL_BOOL;
   }

   return 0;
}

VertexArray::VertexArray()
{
   glCreateVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray()
{
   glDeleteVertexArrays(1, &m_RendererID);
}

void
VertexArray::Bind() const
{
   glBindVertexArray(m_RendererID);
}

void
VertexArray::Unbind() const
{
   glBindVertexArray(0);
}

void
VertexArray::AddVertexBuffer(const std::shared_ptr< VertexBuffer >& vertexBuffer)
{
   glBindVertexArray(m_RendererID);
   vertexBuffer->Bind();

   const auto& layout = vertexBuffer->GetLayout();
   for (const auto& element : layout)
   {
      switch (element.Type)
      {
         case ShaderDataType::Float:
         case ShaderDataType::Float2:
         case ShaderDataType::Float3:
         case ShaderDataType::Float4:
         case ShaderDataType::Int:
         case ShaderDataType::Int2:
         case ShaderDataType::Int3:
         case ShaderDataType::Int4:
         case ShaderDataType::Bool: {
            glEnableVertexAttribArray(m_VertexBufferIndex);
            glVertexAttribPointer(m_VertexBufferIndex, static_cast<GLint>(element.GetComponentCount()),
                                  ShaderDataTypeToOpenGLBaseType(element.Type),
                                  element.Normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(layout.GetStride()),
                                  reinterpret_cast< const void* >(element.Offset)); //NOLINT
            m_VertexBufferIndex++;
            break;
         }
         case ShaderDataType::Mat3:
         case ShaderDataType::Mat4: {
            auto count = element.GetComponentCount();
            for (uint32_t i = 0; i < count; i++)
            {
               glEnableVertexAttribArray(m_VertexBufferIndex);
               glVertexAttribPointer(m_VertexBufferIndex, static_cast<GLint>(count),
                                     ShaderDataTypeToOpenGLBaseType(element.Type),
                                     element.Normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(layout.GetStride()),
                                     reinterpret_cast< const void* >(sizeof(float) * count * i)); //NOLINT
               glVertexAttribDivisor(m_VertexBufferIndex, 1);
               m_VertexBufferIndex++;
            }
            break;
         }
         default:
            break;
      }
   }

   m_VertexBuffers.push_back(vertexBuffer);
}

void
VertexArray::SetIndexBuffer(const std::shared_ptr< IndexBuffer >& indexBuffer)
{
   glBindVertexArray(m_RendererID);
   indexBuffer->Bind();

   m_IndexBuffer = indexBuffer;
}
} // namespace dgame
