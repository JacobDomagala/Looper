#pragma once

#include "logger.hpp"

#include <fmt/core.h>
#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <string>

struct GLFWwindow;

namespace looper::renderer {

// NOLINTNEXTLINE
class Window
{
 public:
   Window(const glm::ivec2& size, const std::string& title);
   ~Window();

   void
   ShutDown();

   void MakeFocus();

   [[nodiscard]] glm::ivec2
   GetSize() const
   {
      return size_;
   }

   void
   Start()
   {
      isRunning_ = true;
   }

   // return true if the window is active
   [[nodiscard]] bool
   IsRunning() const
   {
      return isRunning_;
   }

   void
   Resize(const glm::ivec2& newSize);

   void
   SetIcon(const std::string& file);

   void
   Clear();

   // true -> mouse visible and not wrapped
   // false -> mouse is disabled (hidden and wrapped)
   void
   ShowCursor(bool choice);

   // update and get cursor position <-1, 1>
   // with positive 'y' is up
   glm::vec2
   GetCursorScreenPosition(const glm::mat4& projectionMatrix);

   // update and get cursor position <-1, 1>
   // with positive 'y' is down
   glm::vec2
   GetCursorNormalized();

   // return current cursor position on window
   glm::vec2
   GetCursor();

   GLFWwindow*
   GetWindowHandle()
   {
      return window_;
   }

   explicit operator std::string() const
   {
      return fmt::format("Window title - {} \nWindow dimensions - {}/{}", title_, size_.x, size_.y);
   }

   friend std::ostream&
   operator<<(std::ostream& os, const Window& window)
   {
      os << "Window title - " << window.title_ << "\n Window dimensions - " << window.size_.x << "x"
         << window.size_.y;

      return os;
   }

 private:
   glm::ivec2 size_ = {};
   GLFWwindow* window_ = nullptr;
   std::string title_ = {};

   // is windows active
   bool isRunning_ = {};

   // NOLINTNEXTLINE
   static inline bool GLFW_INIT = false;
};

} // namespace looper::renderer
