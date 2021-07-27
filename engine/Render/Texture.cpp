#include "Texture.hpp"
#include "FileManager.hpp"
#include "Shader.hpp"
#include "Window.hpp"

namespace dgame {

Texture::Texture()
{
   CreateColorTexture({1, 1}, {1, 1, 1});
}

Texture::Texture(const std::string& textureName)
{
   LoadTextureFromFile(textureName);
}

Texture::~Texture()
{
   glDeleteTextures(1, &m_textureID);
}

void
Texture::CreateColorTexture(const glm::ivec2& size, const glm::vec3& /*color*/)
{
   m_width = size.x;
   m_height = size.y;

   const auto sizeArray = static_cast< size_t >(m_width * m_height) * sizeof(byte_vec4);

   m_data = std::make_unique< uint8_t[] >(sizeArray);
   std::memset(m_data.get(), 0xFF, sizeArray);

   LoadTextureFromMemory(size, m_data.get(), "white.png");
}

void
Texture::LoadTextureFromFile(const std::string& fileName, GLenum /*wrapMode*/, GLenum /*filter*/)
{
   auto picture = FileManager::LoadImageData(fileName);

   m_data = std::move(picture.m_bytes);
   m_width = picture.m_size.x;
   m_height = picture.m_size.y;
   m_numChannels = picture.m_format;

   LoadTextureFromMemory({m_width, m_height}, m_data.get(), fileName);
}

void
Texture::LoadTextureFromMemory(const glm::ivec2& size, uint8_t* /*data*/, const std::string& name,
                               GLenum wrapModeS, GLenum wrapModeT, GLenum magFilter,
                               GLenum minFilter)
{
   m_name = name;

   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);

   const auto format = m_numChannels == 4 ? GL_RGBA : GL_RGB;
   glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, GL_UNSIGNED_BYTE,
                m_data.get());

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapModeS);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapModeT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
   glGenerateMipmap(GL_TEXTURE_2D);

   m_logger.Log(Logger::TYPE::DEBUG, "Created new texture {} and bound it to ID {}", m_name,
                m_textureID);
}

void
Texture::Use(GLuint slot)
{
   glBindTextureUnit(slot, m_textureID);
}

GLuint
Texture::Create()
{
   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                m_data.get());
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   return m_textureID;
}

byte_vec4*
Texture::GetVec4Data() const
{
   return reinterpret_cast< byte_vec4* >(GetData());
}

uint8_t*
Texture::GetData() const
{
   return m_data.get();
}

std::string
Texture::GetName() const
{
   return m_name;
}

int32_t
Texture::GetWidth() const
{
   return m_width;
}

int32_t
Texture::GetHeight() const
{
   return m_height;
}

GLuint
Texture::GetTextureHandle() const
{
   return m_textureID;
}

bool
Texture::operator==(const Texture& other) const
{
   return m_textureID == other.GetTextureHandle();
}

} // namespace dgame
