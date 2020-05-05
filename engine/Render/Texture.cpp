#include "FileManager.hpp"
#include <Shaders.hpp>
#include <Texture.hpp>
#include <Window.hpp>

namespace dgame {

Texture::~Texture()
{
   glDeleteTextures(1, &m_textureID);
}

void
Texture::CreateColorTexture(const glm::ivec2& size, const glm::vec3& color)
{
   m_width = size.x;
   m_height = size.y;

   const auto sizeArray = m_width * m_height * sizeof(byte_vec4);

   m_data = std::make_unique< uint8_t[] >(sizeArray);
   std::memset(m_data.get(), 0xFF, sizeArray);

   LoadTextureFromMemory(size, m_data.get(), "NewTexture.png");
}

byte_vec4*
Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
   auto picture = FileManager::LoadImage(fileName);

   m_data = std::move(picture.m_bytes);
   m_width = picture.m_size.x;
   m_height = picture.m_size.y;

   LoadTextureFromMemory({m_width, m_height}, m_data.get(), fileName);

   return reinterpret_cast< byte_vec4* >(GetData());
}

void
Texture::LoadTextureFromMemory(const glm::ivec2& size, uint8_t* data, const std::string& name, GLenum wrapMode, GLenum filter)
{
   m_name = name;

   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data.get());
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxBoundCound);
   // 32 textures can be bound at the same time
   m_unit = m_unitCounter >= m_maxBoundCound ? 0 : m_unitCounter++;

   m_logger.Log(Logger::TYPE::DEBUG, std::string("Created texture " + m_name + " with sampler_ID ") + std::to_string(m_samplerID)
                                        + " using it with unit_id " + std::to_string(m_unit));
}

void
Texture::Use(GLuint program)
{
   GLuint samplerLocation = glGetUniformLocation(program, "texture");

   glActiveTexture(GL_TEXTURE0 + m_unit);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glUniform1i(samplerLocation, m_unit);

   m_nowBound = m_unit;

   ++m_boundCount;
}

GLuint
Texture::Create()
{
   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data.get());
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   return m_textureID;
}

} // namespace dgame