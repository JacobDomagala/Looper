#include "Window.hpp"
#include "Common.hpp"
#include "Game.hpp"
#include "Logger.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>


void
Window::ErrorCallback(int error, const char* description)
{
   fprintf(stderr, "Error: %s\n", description);
}

void GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* logger)
{
   std::string buffer(1024, 0x0);
   const auto newSize = sprintf(&buffer[0], "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
                                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);

   buffer.resize(newSize);
   reinterpret_cast< const Logger* >(logger)->Log(Logger::TYPE::DEBUG, buffer);
}

Window::Window(int32_t width, int32_t height, const std::string& title)
   : m_width(width),
     m_height(height),
     m_title(title),
     m_isRunning(true),
     m_projectionMatrix(glm::ortho(-width / 2.0f, width / 2.0f, height / 2.0f, -height / 2.0f, -1.0f, 1.0f))
{
   m_logger.Init("Window");

   glfwSetErrorCallback(ErrorCallback);

   if (GLFW_TRUE != glfwInit())
   {
      m_logger.Log(Logger::TYPE::FATAL, "GLFW_TRUE != glfwInit()");
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   m_pWindow = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
   m_logger.Log(Logger::TYPE::DEBUG, "GLFW Window created - \n" + std::string(*this));

   glfwMakeContextCurrent(m_pWindow);

   int major, minor;
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   m_logger.Log(Logger::TYPE::DEBUG, "OpenGL Version - " + std::to_string(major) + "." + std::to_string(minor));

   glewExperimental = GL_TRUE;

   if (glewInit() != GLEW_OK)
   {
      m_logger.Log(Logger::TYPE::FATAL, "glewInit() != GLEW_OK");
   }

   int viewportWidth, viewportHeight;
   glfwGetFramebufferSize(m_pWindow, &viewportWidth, &viewportHeight);

   glViewport(0, 0, viewportWidth, viewportHeight);

   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(MessageCallback, &m_logger);

   glfwSwapInterval(1);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_MULTISAMPLE);
}

Window::~Window()
{
   glfwTerminate();
}

void
Window::ShutDown()
{
   m_isRunning = false;
   glfwDestroyWindow(m_pWindow);
}

void
Window::SetIcon(const std::string& file)
{
   Texture texture;

   GLFWimage image;
   image.width = 16;
   image.height = 16;
   image.pixels = reinterpret_cast< unsigned char* >(texture.LoadTextureFromFile(file).get());

   auto cursor = glfwCreateCursor(&image, 0, 0);
   glfwSetCursor(m_pWindow, cursor);
}

void
Window::Resize(int32_t newWidth, int32_t newHeight)
{
   glfwSetWindowSize(m_pWindow, newWidth, newHeight);

   m_height = newHeight;
   m_width = newWidth;
}

void
Window::Clear(float r, float g, float b, float a)
{
   glClearColor(r, g, b, a);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Window::SwapBuffers()
{
   glfwSwapBuffers(m_pWindow);
}

void
Window::ShowCursor(bool choice)
{
   int mode = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
   glfwSetInputMode(m_pWindow, GLFW_CURSOR, mode);
}

glm::vec2
Window::GetCursorScreenPosition()
{
   auto cursorPos = GetCursor();

   cursorPos -= glm::vec2((m_width / 2.0f), (m_height / 2.0f));
   glm::vec4 tmpCursor = m_projectionMatrix * glm::vec4(cursorPos, 0.0f, 1.0f);

   return glm::vec2(tmpCursor.x, tmpCursor.y);
}

glm::vec2
Window::GetCursorNormalized()
{
   auto cursorPos = GetCursor();

   glm::dvec2 centerOfScreen(m_width / 2.0f, m_height / 2.0f);

   cursorPos -= centerOfScreen;
   cursorPos /= centerOfScreen;

   return cursorPos;
}

glm::vec2
Window::GetCursor()
{
   glm::dvec2 cursorPos;
   glfwGetCursorPos(m_pWindow, &cursorPos.x, &cursorPos.y);

   return cursorPos;
}