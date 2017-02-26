#pragma once
#include "Common.h"

class Win_Window {
	WNDCLASS  windowClass;
	HWND      windowHandle;
	HINSTANCE hInstance;
	HGLRC     hrc;
	HDC       hdc;

	glm::mat4 projectionMatrix;
	glm::vec2 cursorPos;
	
public:
	static std::unordered_map<uint8, bool> keyMap;
	//static bool leftMouseKeyPressed;
	//static bool rightMouseKeyPressed;

	bool isRunning;
	int drawWidth;
	int drawHeight;
	int xBias;
	int yBias;

	static LRESULT CALLBACK MainWinProc(HWND hWind, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	Win_Window(HINSTANCE hInstance) :
		hInstance(hInstance) 
	{
		isRunning = true; 
		projectionMatrix = glm::ortho(static_cast<float>(-WIDTH / 2.0f), 
									  static_cast<float>(WIDTH/2.0f),
									  static_cast<float>(HEIGHT/2.0f), 
									  static_cast<float>(-HEIGHT / 2.0f),
									  -1.0f, 1.0f);
	}
	void SetCursor(const glm::vec2 position) { cursorPos = position; }
	glm::vec2 GetCursor() 
	{
		POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(windowHandle, &cursor);

		cursorPos = glm::vec2(cursor.x, cursor.y);

		return glm::vec2(cursor.x, cursor.y);
		//cursorPos.x = static_cast<float>(cursor.x);
		//cursorPos.y = static_cast<float>(cursor.y);
		//
		//return cursorPos;
	}

	glm::vec2 GetCursorScreenPosition()
	{
		POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(windowHandle, &cursor);
		cursorPos = glm::vec2(cursor.x, cursor.y);
		cursorPos -= glm::vec2(static_cast<float>(WIDTH / 2.0f), static_cast<float>(HEIGHT / 2.0f));

		glm::vec4 tmpCursor = projectionMatrix * glm::vec4(cursorPos, 0.0f, 1.0f);
		return glm::vec2(tmpCursor.x, tmpCursor.y);
	}

	glm::vec2 GetCursorNormalized() 
	{
		POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(windowHandle, &cursor);
		cursorPos = glm::vec2(cursor.x, cursor.y);
		
		glm::vec2 center(WIDTH / 2.0f, HEIGHT / 2.0f);
		
		cursor.x -= static_cast<LONG>(center.x);
		cursor.y -= static_cast<LONG>(center.y);
		
		float cursorX = cursor.x / center.x;
		float cursorY = cursor.y / center.y;

		return glm::vec2(cursorX, cursorY);
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



