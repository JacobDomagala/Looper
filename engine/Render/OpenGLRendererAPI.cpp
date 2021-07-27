
#include "OpenGLRendererAPI.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace dgame {

void
OpenGLRendererAPI::Init()
{
   int status = gladLoadGLLoader(reinterpret_cast< GLADloadproc >(glfwGetProcAddress));

   if (status != 1)
   {
      m_logger.Log(Logger::TYPE::FATAL, "gladLoadGLLoader() != OK");
   }

   int major, minor;
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   m_logger.Log(Logger::TYPE::DEBUG, "OpenGL Version - {}.{}", major, minor);

   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

   glDebugMessageCallback(
      [](GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
         const GLchar* message, const void* logger) {
         const auto buffer =
            fmt::format("OpenGL: type = {}, severity = {}, message = {}", type, severity, message);

         auto logg = reinterpret_cast< const Logger* >(logger);

         switch (severity)
         {
            case GL_DEBUG_SEVERITY_HIGH:
               logg->Log(Logger::TYPE::FATAL, buffer);
               return;
            case GL_DEBUG_SEVERITY_MEDIUM:
               logg->Log(Logger::TYPE::WARNING, buffer);
               return;
            case GL_DEBUG_SEVERITY_LOW:
               logg->Log(Logger::TYPE::INFO, buffer);
               return;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
               logg->Log(Logger::TYPE::DEBUG, buffer);
               return;
         }
      },
      &m_logger);

   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL,
                         GL_FALSE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   s_initalized = true;
}

void
OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   glViewport(x, y, width, height);
}

void
OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
   glClearColor(color.r, color.g, color.b, color.a);
}

void
OpenGLRendererAPI::Clear()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
OpenGLRendererAPI::DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray,
                               uint32_t indexCount)
{
   uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
   glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void
OpenGLRendererAPI::DrawLines(uint32_t numLines)
{
   glDrawArrays(GL_LINES, 0, numLines * 2);
}

} // namespace dgame
