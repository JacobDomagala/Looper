#include "Window.hpp"
#include "Common.hpp"
#include "Game.hpp"
#include "Logger.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

void
error_callback(int error, const char* description)
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

Window::Window(uint32_t width, uint32_t height, const std::string& title, Logger& logger)
   : m_width(WIDTH),
     m_height(HEIGHT),
     m_title(title),
     m_isRunning(true),
     m_projectionMatrix(glm::ortho(-WIDTH / 2.0f, WIDTH / 2.0f, HEIGHT / 2.0f, -HEIGHT / 2.0f, -1.0f, 1.0f)),
     m_logger(logger)
{
   glfwSetErrorCallback(error_callback);

   assert(GLFW_TRUE == glfwInit());
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   m_pWindow = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
   m_logger.Log(Logger::TYPE::DEBUG, "GLFW Window created - " + std::to_string(m_width) + "x" + std::to_string(m_height));

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

   // During init, enable debug output
   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(MessageCallback, &logger);

   glfwSwapInterval(1);

   int tmpWidth, tmpHeight;
   glfwGetFramebufferSize(m_pWindow, &tmpWidth, &tmpHeight);

   glViewport(0, 0, tmpWidth, tmpHeight);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_MULTISAMPLE);
}

Window::~Window()
{
   m_isRunning = false;
   glfwTerminate();
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
Window::Resize(uint32_t newWidth, uint32_t newHeight)
{
   // SDL_SetWindowSize(m_pWindow, newWidth, newHeight);

   // m_height = newHeight;
   // m_width  = newWidth;
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
Window::WrapMouse(bool choice)
{
   // SDL_SetWindowGrab(m_pWindow, static_cast<SDL_bool>(choice));
}

void
Window::ShowCursor(bool choice)
{
   // SDL_ShowCursor(choice);
}

glm::vec2
Window::GetCursorScreenPosition()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);

   m_cursorPos = glm::vec2(xpos, ypos);
   m_cursorPos -= glm::vec2((WIDTH / 2.0f), (HEIGHT / 2.0f));
   glm::vec4 tmpCursor = m_projectionMatrix * glm::vec4(m_cursorPos, 0.0f, 1.0f);

   return glm::vec2(tmpCursor.x, tmpCursor.y);
}

glm::vec2
Window::GetCursorNormalized()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);

   glm::vec2 center(m_width / 2.0f, m_height / 2.0f);

   xpos -= center.x;
   ypos -= center.y;

   float cursorX = xpos / center.x;
   float cursorY = ypos / center.y;

   return glm::vec2(cursorX, cursorY);
}

glm::vec2
Window::GetCursor()
{
   double xpos, ypos;
   glfwGetCursorPos(m_pWindow, &xpos, &ypos);

   return glm::vec2(xpos, ypos);
}