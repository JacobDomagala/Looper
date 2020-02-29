#include "InputManager.hpp"
#include "Game.hpp"
#include "Mouse.hpp"

#include <GLFW/glfw3.h>
#include <KeyCodesUtils.hpp>
#include <functional>
#include <iostream>

std::vector< IInputListener* > InputManager::m_inputListeners = {};
glm::vec2 InputManager::m_mousePosition = {};
GLFWwindow* InputManager::m_windowHandle = nullptr;
std::unordered_map< int, bool > InputManager::m_keyMap = {};

void
InputManager::InternalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   Game::GetInstance().Log(Logger::TYPE::DEBUG,
                           "GLFW key " + std::to_string(action) + std::to_string(key) + " scan code - " + std::to_string(scancode));

   m_keyMap[key] = action;

   for (auto& listener : m_inputListeners)
   {
      listener->KeyCallback(key, scancode, action, mods);
   }
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
   Game::GetInstance().Log(Logger::TYPE::DEBUG,
                           "GLFW mouse button " + std::to_string(action) + " " + std::to_string(button) + " " + std::to_string(mods));

   for (auto& listener : m_inputListeners)
   {
      listener->MouseButtonCallback(button, action, mods);
   }
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow* window, double x, double y)
{
   Game::GetInstance().Log(Logger::TYPE::DEBUG, "GLFW cursor pos " + std::to_string(x) + " " + std::to_string(y));

   for (auto& listener : m_inputListeners)
   {
      listener->CursorPositionCallback(x, y);
   }
}

void
InputManager::Init(GLFWwindow* mainWindow)
{
   m_windowHandle = mainWindow;

   glfwSetKeyCallback(m_windowHandle, InternalKeyCallback);
   glfwSetMouseButtonCallback(m_windowHandle, InternalMouseButtonCallback);
   glfwSetCursorPosCallback(m_windowHandle, InternalCursorPositionCallback);
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