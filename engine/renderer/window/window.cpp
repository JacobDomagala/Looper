#include "window.hpp"
#include "common.hpp"
#include "game.hpp"
#include "logger.hpp"
// #include "RenderCommand.hpp"
#include "utils/assert.hpp"

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>

namespace looper::renderer {

Window::Window(const glm::ivec2& size, const std::string& title)
   : size_(size), title_(title), isRunning_(true)
{
   glfwSetErrorCallback([](int error, const char* description) {
      Logger::Fatal("GLFW Error={}: {}", error, description);
   });

   utils::Assert(glfwInit(), "GLFW Init failed!");

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
   glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

   auto* monitor = glfwGetPrimaryMonitor();
   if (size_ == USE_DEFAULT_SIZE)
   {
      const auto* mode = glfwGetVideoMode(monitor);
      size_ = glm::ivec2(mode->width, mode->height);
   }

   window_ = glfwCreateWindow(size_.x, size_.y, title.c_str(), monitor, nullptr);

   utils::Assert(window_, "Failed to create GLFW window!");

   Logger::Info("GLFW Window created! Name:{} Width:{} Height:{}", title_, size_.x, size_.y);
}

Window::~Window()
{
   glfwDestroyWindow(window_);
}

void
Window::ShutDown()
{
   isRunning_ = false;
}

void
Window::SetIcon(const std::string& /*file*/)
{
   GLFWimage image;
   image.width = 16;
   image.height = 16;
   // image.pixels = renderer::TextureLibrary::GetTexture(file)->GetData();

   auto* cursor = glfwCreateCursor(&image, 0, 0);
   glfwSetCursor(window_, cursor);
}

void
Window::Resize(const glm::ivec2& newSize)
{
   glfwSetWindowSize(window_, newSize.x, newSize.y);

   size_ = newSize;
}

void
Window::MakeFocus()
{
   glfwRestoreWindow(window_);
   glfwShowWindow(window_);
   glfwFocusWindow(window_);
}

void
Window::ShowCursor(bool choice)
{
   const int mode = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
   glfwSetInputMode(window_, GLFW_CURSOR, mode);
}

glm::vec2
Window::GetCursorScreenPosition(const glm::mat4& /*projectionMatrix*/)
{
   auto cursorPos = GetCursor();

   cursorPos -=
      glm::vec2((static_cast< float >(size_.x) / 2.0f), (static_cast< float >(size_.y) / 2.0f));
   // glm::vec2 tmpCursor = projectionMatrix * glm::vec4(cursorPos, 0.0f, 1.0f);

   return cursorPos;
}

glm::vec2
Window::GetCursorNormalized()
{
   auto cursorPos = GetCursor();

   const glm::dvec2 centerOfScreen(static_cast< float >(size_.x) / 2.0f,
                                   static_cast< float >(size_.y) / 2.0f);

   cursorPos -= centerOfScreen;
   cursorPos /= centerOfScreen;

   return cursorPos;
}

glm::vec2
Window::GetCursor()
{
   glm::dvec2 cursorPos;
   glfwGetCursorPos(window_, &cursorPos.x, &cursorPos.y);

   return cursorPos;
}

} // namespace looper::renderer
