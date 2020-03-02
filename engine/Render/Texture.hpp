#pragma once

#include <Common.hpp>
#include "Logger.hpp"

#include <GL/glew.h>
#include <string>

class Texture
{

 public:
   Texture() = default;
   ~Texture() = default;

   // DEBUG: number of glBindTexture calls
   static int32_t m_boundCount;

   // Load texture from 'fileName' file and return byte values (used for collision)
   std::unique_ptr< byte_vec4 >
   LoadTextureFromFile(const std::string& fileName = "Assets//Default.png", GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);

   // Load texture from 'data' memory
   void
   LoadTextureFromMemory(int32_t width, int32_t height, byte_vec4* data, GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);

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
   static int32_t m_unitCounter;

   // ID of currently bound texture
   static int32_t m_nowBound;

   // texture unit
   int32_t m_unit;

   Logger m_logger = Logger("Texture");

};
