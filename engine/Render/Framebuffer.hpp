#pragma once

#include "Common.hpp"
#include "Shaders.hpp"

#include <GL/glew.h>
#include <array>
#include <memory>

namespace dgame {

class Framebuffer
{
 public:
   Framebuffer() = default;
   ~Framebuffer() = default;

   void
   SetUp();

   void
   LoadShaders(const std::string& shaderName);

   void
   BeginDrawingToTexture();

   void
   EndDrawingToTexture();

   void
   DrawFrameBuffer();

 private:
   uint32_t m_currentFrame = 0;
   GLuint m_framebufferID;
   GLuint m_textureID;
   Shaders m_shaders;


   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};
};

} // namespace dgame