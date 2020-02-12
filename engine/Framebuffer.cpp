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
}

void
Framebuffer::LoadShaders(const std::string& shaderName)
{
   shaders.LoadShaders("../Shaders/" + shaderName + "_vs.glsl", "../Shaders/" + shaderName + "_fs.glsl");
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

   shaders.UseProgram();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, textureID);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}
