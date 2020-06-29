#pragma once

#include "Common.hpp"
#include "Shader.hpp"

#include <glad/glad.h>
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
   GLuint m_framebufferID = 0;
   GLuint m_textureID = 0;

   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};
};

} // namespace dgame