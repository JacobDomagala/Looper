#include "InputManager.hpp"
#include "Game.hpp"
#include "Mouse.hpp"

#include <GLFW/glfw3.h>
#include <KeyCodesUtils.hpp>
#include <functional>
#include <iostream>

namespace dgame {

void
InputManager::InternalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   m_logger.Log(Logger::TYPE::TRACE,
                "GLFW key " + std::to_string(action) + std::to_string(key) + " scan code - " + std::to_string(scancode));

   m_keyMap[key] = action;
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
   m_logger.Log(Logger::TYPE::TRACE,
                "GLFW mouse button " + std::to_string(action) + " " + std::to_string(button) + " " + std::to_string(mods));
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow* window, double x, double y)
{
   m_logger.Log(Logger::TYPE::TRACE, "GLFW cursor pos " + std::to_string(x) + " " + std::to_string(y));

   m_mousePosition = glm::vec2(x, y);
}

void
InputManager::Init(GLFWwindow* mainWindow)
{
   m_windowHandle = mainWindow;
   m_logger.Init("InputManager");

   glfwSetKeyCallback(m_windowHandle, InternalKeyCallback);
   //  glfwSetMouseButtonCallback(m_windowHandle, InternalMouseButtonCallback);
   //  glfwSetCursorPosCallback(m_windowHandle, InternalCursorPositionCallback);

   m_keyMap.clear();
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

bool
InputManager::CheckKeyPressed(int keyKode)
{
   return m_keyMap[keyKode];
}

} // namespace dgame