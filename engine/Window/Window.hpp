#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

struct GLFWwindow;
class Logger;

class Window
{
public:
   Window(uint32_t width, uint32_t height, const std::string& title, Logger& logger);
	~Window();

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

	// return true if the window is active
   bool
   IsRunning() const
   {
      return m_isRunning;
   }

	void
	Resize(uint32_t newWidth, uint32_t newHeight);

	void
	SetIcon(const std::string& file);

	void
	Clear(float r, float g, float b, float a);

	void
	SwapBuffers();

	void
	WrapMouse(bool choice);

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

   // set cursos position
   void
   SetCursor(const glm::vec2& position)
   {
      m_cursorPos = position;
   }

   // update and get curson position in Win32 coords
   glm::vec2
   GetCursor();

private:
   uint32_t    m_width;
   uint32_t    m_height;
   GLFWwindow*  m_pWindow;
   std::string m_title;

   // cursor position
   glm::vec2 m_cursorPos;

   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;

   // is windows active
   bool m_isRunning;

   Logger& m_logger;
};

