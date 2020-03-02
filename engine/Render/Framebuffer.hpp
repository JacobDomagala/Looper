#pragma once

#include "Common.hpp"
#include "Shaders.hpp"

#include <GL/glew.h>
#include <array>

class Framebuffer
{
 public:
   Framebuffer() = default;
   ~Framebuffer() = default;

   void SetUp();

   void
   LoadShaders(const std::string& shaderName);

   void
   BeginDrawingToTexture();

   void
   EndDrawingToTexture();

   void
   DrawFrameBuffer();

   void
   DrawPreviousFrameBuffer();

private:
   uint32_t m_currentFrame = 1;
   std::array<GLuint, NUM_FRAMES_TO_SAVE> m_frameBufferIDs;
   std::array<GLuint, NUM_FRAMES_TO_SAVE> m_textureIDs;
   Shaders shaders;

   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};

};
