#include "InputManager.hpp"
#include "Mouse.hpp"
#include "Game.hpp"

#include <functional>
#include <iostream>
#include <KeyCodesUtils.hpp>
#include <GLFW/glfw3.h>

// STATIC GLFW CALLBACKS FOR INPUT
void InputManager::InternalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Game::GetInstance().Log(Logger::TYPE::DEBUG, "GLFW key " + std::to_string(action) + std::to_string(key) + " scan code - " + std::to_string(scancode));
}

void InputManager::InternalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Game::GetInstance().Log(Logger::TYPE::DEBUG, "GLFW mouse button " + std::to_string(action) + " " + std::to_string(button) + " " + std::to_string(mods));
}

void InputManager::InternalMousePositionCallback(GLFWwindow* window, double x, double y)
{
	Game::GetInstance().Log(Logger::TYPE::DEBUG, "GLFW cursor pos " + std::to_string(x) + " " + std::to_string(y));
}


// NORMAL CLASS FUNCTIONS
void
InputManager::Init(GLFWwindow* mainWindow)
{
	m_windowHandle = mainWindow;

	glfwSetKeyCallback(m_windowHandle, InternalKeyCallback);
	glfwSetMouseButtonCallback(m_windowHandle, InternalMouseButtonCallback);
	glfwSetCursorPosCallback(m_windowHandle, InternalMousePositionCallback);
}

void
InputManager::RegisterForKeyInput(IInputListener* listener)
{
	m_inputListeners.push_back(listener);
}

void
InputManager::HandleInput()
{
	glfwPollEvents();
}
