#include "Framebuffer.hpp"
#include "Common.hpp"
#include "Texture.hpp"

namespace dgame {

void
Framebuffer::SetUp()
{
   glGenFramebuffers(1, &m_framebufferID);
   glGenTextures(1, &m_textureID);

   glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glActiveTexture(GL_TEXTURE0);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

   GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
   glDrawBuffers(1, drawBuffers);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glBindTexture(GL_TEXTURE_2D, 0);

   m_shaders.LoadShaders("AfterEffects");

   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   // Each vec4 is Position, TexCoord
   glm::vec4 positions[4] = {glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f),
                             glm::vec4(1.0f, -1.0f, 1.0f, 0.0f)};

   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);
}

void
Framebuffer::LoadShaders(const std::string& shaderName)
{
   m_shaders.LoadShaders(shaderName);
}

void
Framebuffer::BeginDrawingToTexture()
{
   // DRAW TO FRAMEBUFFER
   glBindFramebuffer(GL_FRAMEBUFFER, m_textureID);
   glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}

void
Framebuffer::EndDrawingToTexture()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Framebuffer::DrawFrameBuffer()
{
   glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   m_shaders.UseProgram();
   glBindVertexArray(m_vertexArrayBuffer);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   // glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, m_texturesBytes.at(m_currentFrame).get());

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   glBindVertexArray(0);

   // m_currentFrame = m_currentFrame >= NUM_FRAMES_TO_SAVE ? 0 : m_currentFrame++;
}

} // namespace dgame