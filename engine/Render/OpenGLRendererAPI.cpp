
#include "OpenGLRendererAPI.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace looper {

void
OpenGLRendererAPI::Init()
{
   // NOLINTNEXTLINE
   int status = gladLoadGLLoader(reinterpret_cast< GLADloadproc >(glfwGetProcAddress));

   if (status != 1)
   {
      Logger::Fatal("OpenGLRendererAPI: gladLoadGLLoader() != OK");
   }

   int major = {};
   int minor = {};
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   Logger::Debug("OpenGLRendererAPI: OpenGL Version - {}.{}", major, minor);

   glEnable(GL_DEBUG_OUTPUT);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

   glDebugMessageCallback(
      [](GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
         const GLchar* message, const void* /**/) {

         switch (severity)
         {
            case GL_DEBUG_SEVERITY_HIGH:
               Logger::Fatal("OpenGL: type = {}, severity = {}, message = {}", type,
                             severity, message);
               return;
            case GL_DEBUG_SEVERITY_MEDIUM:
               Logger::Warn("OpenGL: type = {}, severity = {}, message = {}", type, severity,
                            message);
               return;
            case GL_DEBUG_SEVERITY_LOW:
               Logger::Info("OpenGL: type = {}, severity = {}, message = {}", type, severity,
                            message);
               return;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
            default:
               Logger::Debug("OpenGL: type = {}, severity = {}, message = {}", type, severity,
                             message);
               return;
         }
      },
      nullptr);

   glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                         GL_FALSE);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   s_initalized = true;
}

void
OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   glViewport(static_cast< GLint >(x), static_cast< GLint >(y), static_cast< GLint >(width),
              static_cast< GLint >(height));
}

void
OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
   glClearColor(color.r, color.g, color.b, color.a);
}

void
OpenGLRendererAPI::Clear()
{
   // NOLINTNEXTLINE
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
OpenGLRendererAPI::DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray,
                               uint32_t indexCount)
{
   uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
   glDrawElements(GL_TRIANGLES, static_cast< GLsizei >(count), GL_UNSIGNED_INT, nullptr);
   glBindTexture(GL_TEXTURE_2D, 0);
}

void
OpenGLRendererAPI::DrawLines(uint32_t indexCount)
{
   glDrawArrays(GL_LINES, 0, static_cast< GLsizei >(indexCount * 2));
}

} // namespace looper
