#pragma once

#include "Logger.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <string>

struct GLFWwindow;

class Window
{
 public:
   Window(int32_t width, int32_t height, const std::string& title);
   ~Window();

   void
   ShutDown();

   // set projection matrix for OpenGL
   void
   SetProjection(const glm::mat4& projection)
   {
      m_projectionMatrix = projection;
   }

   // get projection matrix
   const glm::mat4&
   GetProjection() const
   {
      return m_projectionMatrix;
   }

   const glm::vec2&
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
   bool
   IsRunning() const
   {
      return m_isRunning;
   }

   void
   Resize(int32_t newWidth, int32_t newHeight);

   void
   SetIcon(const std::string& file);

   void
   Clear(float r, float g, float b, float a);

   void
   SwapBuffers();

   // true -> mouse visible and not wrapped
   // false -> mouse is disabled (hidden and wrapped)
   void
   ShowCursor(bool choice);

   // update and get cursor position <-1, 1>
   // with positive 'y' is up
   glm::vec2
   GetCursorScreenPosition();

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

   operator std::string() const
   {
      std::stringstream returnStr;
      returnStr << "Window title - " << m_title << '\n' << "Window dimensions - " << m_width << "x" << m_height;

      return returnStr.str();
   }

   friend std::ostream&
   operator<<(std::ostream& os, Window window)
   {
      os << "Window title - " << window.m_title << '\n' << "Window dimensions - " << window.m_width << "x" << window.m_height;

      return os;
   }

   static void
   ErrorCallback(int error, const char* description);

 private:
   uint32_t m_width;
   uint32_t m_height;
   GLFWwindow* m_pWindow = nullptr;
   std::string m_title;

   // cursor position
   glm::vec2 m_cursorPos;

   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;

   // is windows active
   bool m_isRunning;

   Logger m_logger;
};
