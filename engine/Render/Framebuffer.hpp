#pragma once

#include "Common.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace dgame {

class Framebuffer
{
 public:
   Framebuffer() = default;
   ~Framebuffer() = default;

   void
   SetUp(const glm::ivec2& windowSize);

   void
   BeginDrawingToTexture();

   void
   EndDrawingToTexture();

   GLuint
   GetTExtureID() const;

 private:
   GLuint m_framebufferID = 0;
   GLuint m_textureID = 0;
};

} // namespace dgame