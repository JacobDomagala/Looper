#include <Shaders.hpp>
#include <Texture.hpp>
#include <Window.hpp>
#include <stb_image.h>

int32_t Texture::m_unitCounter = 0;
int32_t Texture::m_nowBound = 0;
int32_t Texture::m_boundCount = 0;

std::unique_ptr< byte_vec4 >
Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
   int32_t n = 0;
   std::unique_ptr< byte_vec4 > data(reinterpret_cast< byte_vec4* >(stbi_load(fileName.c_str(), &m_width, &m_height, &n, 0)));

   if (!data)
   {
      m_logger.Log(Logger::TYPE::FATAL, std::string("Can't load the file ") + fileName);
   }

   LoadTextureFromMemory(m_width, m_height, data.get());

   return data;
}

void
Texture::LoadTextureFromMemory(int32_t width, int32_t height, byte_vec4* data, GLenum wrapMode, GLenum filter)
{
   glGenTextures(1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast< uint8_t* >(data));
   glGenerateMipmap(GL_TEXTURE_2D);

   // glTexParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, filter);
   // glTexParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, filter);
   // glTexParameteri(m_textureID, GL_TEXTURE_WRAP_S, wrapMode);
   // glTexParameteri(m_textureID, GL_TEXTURE_WRAP_T, wrapMode);

   m_unit =m_textureID;// m_unitCounter++;

   m_logger.Log(Logger::TYPE::DEBUG, std::string("Created texture with sampler_ID ") + std::to_string(m_samplerID) + " using it with unit_id " + std::to_string(m_unit));
}

void
Texture::Use(GLuint program)
{
   if (m_nowBound != m_unit)
   {
      m_logger.Log(Logger::TYPE::DEBUG, std::string("Binding texture with ID ") + std::to_string(m_unit) + " for shader program " + std::to_string(program));

      GLuint samplerLocation = glGetUniformLocation(program, "texture");

      glActiveTexture(GL_TEXTURE0 + m_unit);
      glBindTexture(GL_TEXTURE_2D, m_textureID);
      glUniform1i(samplerLocation, m_unit);

      m_nowBound = m_unit;

      ++m_boundCount;
   }
   else
   {
      m_logger.Log(Logger::TYPE::DEBUG, std::string("Using already bound texture with ID ") + std::to_string(m_unit) + " for shader program " + std::to_string(program));
   }

}
