#include "Framebuffer.hpp"
#include "Common.hpp"
#include "Texture.hpp"

namespace dgame {

void
Framebuffer::SetUp(const glm::ivec2& windowSize)
{
   if (m_framebufferID)
   {
      glDeleteFramebuffers(1, &m_framebufferID);
      glDeleteTextures(1, &m_textureID);
   }

   glCreateFramebuffers(1, &m_framebufferID);
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

   glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Framebuffer::BeginDrawingToTexture()
{
   // DRAW TO FRAMEBUFFER
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
   glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}

void
Framebuffer::EndDrawingToTexture()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint
Framebuffer::GetTExtureID() const
{
   return m_textureID;
}

} // namespace dgame