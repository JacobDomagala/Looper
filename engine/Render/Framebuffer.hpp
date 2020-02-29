#pragma once

#include <GL/glew.h>
#include <Shaders.hpp>

class Framebuffer
{
   GLuint frameBufferID;
   GLuint textureID;
   Shaders shaders;

   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};

 public:
   Framebuffer();
   ~Framebuffer() = default;

   void
   LoadShaders(const std::string& shaderName);

   void
   BeginDrawingToTexture();

   void
   EndDrawingToTexture();

   void
   DrawFrameBuffer();
};
