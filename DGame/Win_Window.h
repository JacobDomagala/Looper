#pragma once
#include"Common.h"


class Win_Window {
private:
	WNDCLASS  windowClass;
	HWND      windowHandle;
	HINSTANCE hInstance;
	HGLRC     hrc;
	HDC       hdc;

	glm::mat4 projectionMatrix;
	
	
	
public:
	static bool isRunning;
	int drawWidth;
	int drawHeight;
	int xBias;
	int yBias;
	Win_Window(HINSTANCE hInstance) : hInstance(hInstance) 
	{
		isRunning = true; 
		projectionMatrix = glm::ortho(static_cast<float>(-WIDTH / 2.0f), 
									  static_cast<float>(WIDTH/2.0f),
									  static_cast<float>(HEIGHT/2.0f), 
									  static_cast<float>(-HEIGHT / 2.0f),
									  -1.0f, 1.0f);
	}
	void Createwindow();
	void SetUpOpenGL();

	const glm::mat4& GetProjection() { return projectionMatrix; }
	void SetProjection(const glm::mat4& projection) { projectionMatrix = projection; }

	void ShutDown() 
	{ 
		isRunning = false; 
		DestroyWindow(windowHandle);
		exit(EXIT_SUCCESS);
	}
	
	void Swapwindow() { SwapBuffers(hdc); }
	HWND GetWindowHandle() { return windowHandle; }
	void ShowError(const std::string& errorMessage, const std::string& errorTitle)
	{
		MessageBoxExA(windowHandle, errorMessage.c_str(), errorTitle.c_str(), MB_OK, 0);
		ShutDown();
	}
};



