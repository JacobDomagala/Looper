#include"Win_Window.h"

std::unordered_map<unsigned char, bool> Win_Window::keyMap;
//bool Win_Window::leftMouseKeyPressed;
//bool Win_Window::rightMouseKeyPressed;

LRESULT CALLBACK Win_Window::MainWinProc(HWND hWind, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT returnVal = 0;
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		keyMap[wParam] = true;
	}break;
	case WM_KEYUP:
	{
		keyMap[wParam] = false;
	}break;
	case WM_LBUTTONDOWN:
	{
		keyMap[wParam] = true;
	}
	break;
	case WM_LBUTTONUP:
	{
		keyMap[wParam] = false;
	}
	break;
	case WM_RBUTTONDOWN:
	{
		keyMap[wParam] = true;
	}
	break;
	case WM_RBUTTONUP:
	{
		keyMap[wParam] = true;
	}
	break;
	case WM_SIZE:
	{
	}
	break;
	case WM_DESTROY:
	{
	}
	break;
	default:
		returnVal = DefWindowProc(hWind, uMsg, wParam, lParam);
	}
	
	return returnVal;
}

void Win_Window::Createwindow()
{	
	windowClass = { 0 };
	//windowClass.style = CS_HREDRAW;
	windowClass.hInstance = hInstance;
	windowClass.lpfnWndProc = MainWinProc;
	windowClass.hCursor = LoadCursorW(NULL, IDC_CROSS);
	windowClass.lpszClassName = L"className";
	
	RegisterClassW(&windowClass);

	DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
	RECT rect = { 0 };
	rect.right = WIDTH;
	rect.bottom = HEIGHT;
	AdjustWindowRectEx(&rect, windowStyle, NULL, NULL);
	drawWidth = rect.right - rect.left;
	drawHeight = rect.bottom - rect.top;
	
	xBias = rect.left;
	yBias = -rect.top;
	
	windowHandle = CreateWindowExA(0, "className", "windowName", windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
								   rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, 0);
	GetClientRect(windowHandle, &rect);
	int i = 6;
}
void Win_Window::SetUpOpenGL()
{
	hdc = GetDC(windowHandle);

	PIXELFORMATDESCRIPTOR pixelDescriptor;
	pixelDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDescriptor.nVersion = 1;
	pixelDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelDescriptor.iLayerType = PFD_MAIN_PLANE;
	pixelDescriptor.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER ;
	pixelDescriptor.cDepthBits = 0;
	pixelDescriptor.cColorBits = 32;
	pixelDescriptor.cAlphaBits = 8;
	

	int pixelformat = ChoosePixelFormat(hdc, &pixelDescriptor);
	PIXELFORMATDESCRIPTOR tmp;
	DescribePixelFormat(hdc, pixelformat, sizeof(tmp), &tmp);
	SetPixelFormat(hdc, pixelformat, &tmp);
	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	if (glewInit() != GLEW_OK) 
		ShowError("Can't initialize OpenGL!", "OpenGL error");
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
	glViewport(0, 0, WIDTH, HEIGHT);
	
	//glDisable(GL_DEPTH_TEST);
}

