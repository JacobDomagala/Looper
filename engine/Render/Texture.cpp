#include "FileManager.hpp"
#include <Shaders.hpp>
#include <Texture.hpp>
#include <Window.hpp>

void
Texture::CreateColorTexture(const glm::vec2& size, const glm::vec3& color)
{
   std::unique_ptr< byte_vec4[] > data(new byte_vec4[size.x * size.y]);
   std::memset(data.get(), 0xFF, size.x * size.y * 4);
   LoadTextureFromMemory(size.x, size.y, reinterpret_cast< uint8_t* >(data.get()));
}

std::unique_ptr< byte_vec4 >
Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
   // int32_t n = 0;
   // std::unique_ptr< byte_vec4 > data(reinterpret_cast< byte_vec4* >(stbi_load(fileName.c_str(), &m_width, &m_height, &n, 0)));

   auto picture = FileManager::LoadPictureRawData(fileName);
   m_width = picture.m_size.x;
   m_height = picture.m_size.y;

   LoadTextureFromMemory(m_width, m_height, picture.m_bytes);

   std::unique_ptr< byte_vec4 > data(reinterpret_cast< byte_vec4* >(picture.m_bytes));

   return data;
}

void
Texture::LoadTextureFromMemory(int32_t width, int32_t height, uint8_t* data, GLenum wrapMode, GLenum filter)
{
   m_data = data;

   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast< uint8_t* >(m_data));
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // 32 textures can be bound at the same time
   m_unit = m_unitCounter >= m_maxBoundCound ? 0 : m_unitCounter++;

   m_logger.Log(Logger::TYPE::DEBUG, std::string("Created texture with sampler_ID ") + std::to_string(m_samplerID)
                                        + " using it with unit_id " + std::to_string(m_unit));
}

void
Texture::Use(GLuint program)
{
   m_logger.Log(Logger::TYPE::DEBUG,
                std::string("Binding texture with ID ") + std::to_string(m_unit) + " for shader program " + std::to_string(program));

   GLuint samplerLocation = glGetUniformLocation(program, "texture");

   glActiveTexture(GL_TEXTURE0 + m_unit);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glUniform1i(samplerLocation, m_unit);

   m_nowBound = m_unit;

   ++m_boundCount;
}
