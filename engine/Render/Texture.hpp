#pragma once

#include "Common.hpp"
#include "FileManager.hpp"
#include "Logger.hpp"

#include <GL/glew.h>
#include <memory>
#include <string>

class Texture
{
 public:
   Texture() = default;
   ~Texture() = default;

   // DEBUG: number of glBindTexture calls
   static inline int32_t m_boundCount = 0;

   // Create new texture which is filled with 'color'
   void
   CreateColorTexture(const glm::ivec2& size, const glm::vec3& color);

   // Load texture from 'fileName' file and return byte values (used for collision)
   byte_vec4*
   LoadTextureFromFile(const std::string& fileName = "Default.png", GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);

   uint8_t*
   GetData()
   {
      return m_data.get();
   }

   std::string
   GetName() const
   {
      return m_name;
   }

   int32_t
   GetWidth() const
   {
      return m_width;
   }

   int32_t
   GetHeight() const
   {
      return m_height;
   }

   GLuint
   GetTextureHandle() const
   {
      return m_textureID;
   }

   // Make this texture active for given 'programID'
   void
   Use(GLuint programID);

 private:
   // Load texture from 'data' memory
   void
   LoadTextureFromMemory(const glm::ivec2& size, uint8_t* data, const std::string& name, GLenum wrapMode = GL_REPEAT,
                         GLenum filter = GL_LINEAR);

 private:
   FileManager::ImageHandleType m_data;

   // width and size of texture
   int32_t m_width = 0;
   int32_t m_height = 0;

   std::string m_name = "EmptyName.png";

   // texture ID used in
   GLuint m_textureID = 0;

   // sampler ID
   GLuint m_samplerID = 0;

   // each time new texture is loaded this counter is increased
   static inline int32_t m_unitCounter = 0;

   // ID of currently bound texture
   static inline int32_t m_nowBound = 0;

   GLint m_maxBoundCound = 31;

   // texture unit
   int32_t m_unit = 0;

   Logger m_logger = Logger("Texture");
};
