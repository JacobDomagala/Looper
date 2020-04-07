#pragma once

#include "Logger.hpp"
#include <Common.hpp>

#include <GL/glew.h>
#include <string>

class Texture
{
 public:
   Texture() = default;
   ~Texture() = default;

   // DEBUG: number of glBindTexture calls
   static inline int32_t m_boundCount = 0;
   
   void
   CreateColorTexture(const glm::vec2& size, const glm::vec3& color);

   // Load texture from 'fileName' file and return byte values (used for collision)
   std::unique_ptr< byte_vec4 >
   LoadTextureFromFile(const std::string& fileName = "Assets//Default.png", GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);

   // Load texture from 'data' memory
   void
   LoadTextureFromMemory(int32_t width, int32_t height, uint8_t* data, GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);

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
   // width and size of texture
   int32_t m_width;
   int32_t m_height;

   // texture ID used in
   GLuint m_textureID;

   // sampler ID
   GLuint m_samplerID;

   // each time new texture is loaded this counter is increased
   static inline int32_t m_unitCounter = 0;

   // ID of currently bound texture
   static inline int32_t m_nowBound = 0;

   GLuint m_maxBoundCound = 31;

   // texture unit
   int32_t m_unit;

   Logger m_logger = Logger("Texture");

   // FIX LATER -> THIS IS RAW POINTER FROM UNIQUE
   uint8_t* m_data;
};
