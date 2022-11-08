#include "Window.hpp"
#include "Common.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "RenderCommand.hpp"
#include "Utils/assert.hpp"

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

namespace looper {

Window::Window(int32_t width, int32_t height, const std::string& title)
   : m_width(width), m_height(height), m_title(title), m_isRunning(true)
{
   glfwSetErrorCallback([](int error, const char* description) {
      Logger::Fatal("GLFW Error={}: {}", error, description);
   });

   utils::Assert(glfwInit(), "GLFW Init failed!");

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   m_pWindow = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);

   utils::Assert(m_pWindow, "Failed to create GLFW window!");

   Logger::Info("GLFW Window created! Name:{} Width:{} Height:{}", m_title, m_width,
                       m_height);
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
   GLFWimage image;
   image.width = 16;
   image.height = 16;
   image.pixels = TextureLibrary::GetTexture(file)->GetData();

   auto* cursor = glfwCreateCursor(&image, 0, 0);
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
Window::Clear()
{
   glfwMakeContextCurrent(m_pWindow);

   RenderCommand::Clear();
}

void
Window::SwapBuffers()
{
   if (m_isRunning)
   {
      glfwMakeContextCurrent(m_pWindow);
      glfwSwapBuffers(m_pWindow);
   }
}

void
Window::ShowCursor(bool choice)
{
   int mode = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
   glfwSetInputMode(m_pWindow, GLFW_CURSOR, mode);
}

glm::vec2
Window::GetCursorScreenPosition(const glm::mat4& /*projectionMatrix*/)
{
   auto cursorPos = GetCursor();

   cursorPos -=
      glm::vec2((static_cast< float >(m_width) / 2.0f), (static_cast< float >(m_height) / 2.0f));
   // glm::vec2 tmpCursor = projectionMatrix * glm::vec4(cursorPos, 0.0f, 1.0f);

   return cursorPos;
}

glm::vec2
Window::GetCursorNormalized()
{
   auto cursorPos = GetCursor();

   glm::dvec2 centerOfScreen(static_cast< float >(m_width) / 2.0f,
                             static_cast< float >(m_height) / 2.0f);

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

} // namespace dgame
