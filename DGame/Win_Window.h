#pragma once

#include "Common.h"

class Win_Window 
{
	// Win32 specific members
	WNDCLASS  m_windowClass;
	HWND      m_windowHandle;
	HINSTANCE m_hInstance;
	HGLRC     m_hrc;
	HDC       m_hdc;

	// projection matrix for OpenGL
	glm::mat4 m_projectionMatrix;

	// cursor position 
	glm::vec2 m_cursorPos;

	// is windows active
	bool m_isRunning;

	// key map
	static std::unordered_map<WPARAM, bool> keyMap;

	// private constructor
	Win_Window(HINSTANCE hInstance);

public:
	// singleton for window
	static Win_Window* GetInstance();

	// Win32 callback function
	static LRESULT CALLBACK MainWinProc(HWND hWind, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// deleted copy constructor
	Win_Window(Win_Window&) = delete;

	// return true if given key is pressed
	static bool GetKeyState(WPARAM keyValue)
	{
		return keyMap[keyValue];
	}

	// return true if the window is active
	bool IsRunning() const
	{
		return m_isRunning;
	}

	// set cursos position
	void SetCursor(const glm::vec2 position) 
	{ 
		m_cursorPos = position; 
	}

	// update and get curson position in Win32 coords
	glm::vec2 GetCursor();
	
	// update and get cursor position <-1, 1> 
	// with positive 'y' is up
	glm::vec2 GetCursorScreenPosition();

	// update and get cursor position <-1, 1>
	// with positive 'y' is down
	glm::vec2 GetCursorNormalized();

	// create Win32 window
	void Createwindow();

	// initialize OpenGL
	void SetUpOpenGL();

	// set projection matrix for OpenGL
	void SetProjection(const glm::mat4& projection) 
	{ 
		m_projectionMatrix = projection; 
	}

	// get projection matrix
	const glm::mat4& GetProjection() const
	{ 
		return m_projectionMatrix; 
	}

	// get window handle
	HWND GetWindowHandle()
	{
		return m_windowHandle;
	}

	// destroy window
	void ShutDown();
	
	// swap render buffers
	void Swapwindow();

	// create pop-up message box with error and shut down 
	void ShowError(const std::string& errorMessage, const std::string& errorTitle);
};



