#include <Common.hpp>
#include <Framebuffer.hpp>

Framebuffer::Framebuffer()
{
   glGenFramebuffers(1, &frameBufferID);
   glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

   glGenTextures(1, &textureID);
   glBindTexture(GL_TEXTURE_2D, textureID);
   glActiveTexture(GL_TEXTURE0);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

   GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
   glDrawBuffers(1, drawBuffers);

   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glBindTexture(GL_TEXTURE_2D, 0);

   shaders.LoadShaders("AfterEffects_vs.glsl", "AfterEffects_fs.glsl");

   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

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
   shaders.LoadShaders(shaderName + "_vs.glsl", shaderName + "_fs.glsl");
}

void
Framebuffer::BeginDrawingToTexture()
{
   // DRAW TO FRAMEBUFFER
   glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
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

   shaders.UseProgram();
   glBindVertexArray(m_vertexArrayBuffer);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, textureID);

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   glBindVertexArray(0);
}
