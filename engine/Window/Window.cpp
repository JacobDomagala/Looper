#include "Window.hpp"
#include <GL/glew.h>
#include <SDL.h>
#include <functional>

namespace noc::window
{

Window::Window(uint32_t width, uint32_t height, const std::string& title)
   : m_width(width)
   , m_height(height)
   , m_title(title)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	m_pWindow = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_OPENGL);
	SDL_GL_GetCurrentContext();
	SDL_GLContext glContext = SDL_GL_CreateContext(m_pWindow);

	GLenum status = glewInit();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

Window::~Window()
{
	SDL_DestroyWindow(m_pWindow);
	SDL_Quit();
}

void
Window::SetIcon(const std::string& file)
{
	auto image   = SDL_RWFromFile(file.c_str(), "rb");
	auto surface = SDL_LoadBMP_RW(image, 1);

	SDL_SetWindowIcon(m_pWindow, surface);
	SDL_FreeSurface(surface);
}

void
Window::Resize(uint32_t newWidth, uint32_t newHeight)
{
	SDL_SetWindowSize(m_pWindow, newWidth, newHeight);

	m_height = newHeight;
	m_width  = newWidth;
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
	SDL_GL_SwapWindow(m_pWindow);
}

void
Window::WrapMouse(bool choice)
{
	SDL_SetWindowGrab(m_pWindow, static_cast<SDL_bool>(choice));
}

void
Window::ShowCursor(bool choice)
{
	SDL_ShowCursor(choice);
}

}   // namespace noc::window
