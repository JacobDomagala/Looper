#pragma once

#include "logger.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <string>

struct GLFWwindow;

namespace looper {

// NOLINTNEXTLINE
class Window
{
 public:
   Window(int32_t width, int32_t height, const std::string& title);

   void
   ShutDown();

   [[nodiscard]] glm::ivec2
   GetSize() const
   {
      return {m_width, m_height};
   }

   void
   Start()
   {
      m_isRunning = true;
   }

   // return true if the window is active
   [[nodiscard]] bool
   IsRunning() const
   {
      return m_isRunning;
   }

   void
   Resize(int32_t newWidth, int32_t newHeight);

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
      return m_pWindow;
   }

   explicit operator std::string() const
   {
      std::stringstream returnStr;
      returnStr << "Window title - " << m_title << "\nWindow dimensions - " << m_width << "x"
                << m_height;

      return returnStr.str();
   }

   friend std::ostream&
   operator<<(std::ostream& os, const Window& window)
   {
      os << "Window title - " << window.m_title << "\n Window dimensions - " << window.m_width
         << "x" << window.m_height;

      return os;
   }

 private:
   int32_t m_width = {};
   int32_t m_height = {};
   GLFWwindow* m_pWindow = nullptr;
   std::string m_title = {};

   // cursor position
   // glm::vec2 m_cursorPos;

   // is windows active
   bool m_isRunning = {};

   // NOLINTNEXTLINE
   static inline bool s_glfwInitalized = false;
};

} // namespace dgame
