#pragma once

#include "Common.hpp"
#include "Shaders.hpp"

#include <GL/glew.h>
#include <array>
#include <memory>

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

   void
   DrawPreviousFrameBuffer();

 private:
   uint32_t m_currentFrame = 0;
   GLuint m_framebufferID;
   GLuint m_textureID;
   // std::array<GLuint, NUM_FRAMES_TO_SAVE> m_frameBufferIDs;
   std::array< std::unique_ptr< uint8_t[] >, NUM_FRAMES_TO_SAVE > m_texturesBytes;
   Shaders m_shaders;


   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};
};
