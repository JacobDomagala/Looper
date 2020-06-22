#include "InputManager.hpp"
#include "Event.hpp"

#include <glfw/glfw3.h>
#include <iostream>

namespace dgame {

void
InputManager::InternalKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW key {} {} scan code - {}", action, key, scancode);

   s_keyMap[key] = action;

   BroadcastEvent(EventType::KEY, KeyEvent{key, scancode, action, mods});
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW mouse button {} {} {}", button, action, mods);

   BroadcastEvent(EventType::MOUSE_BUTTON, MouseButtonEvent{button, action, mods});
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow* window, double x, double y)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW cursor pos {} {}", x, y);

   s_mousePosition = glm::vec2(x, y);

   BroadcastEvent(EventType::MOUSE_CURSOR, CursorPositionEvent{x, y});
}

void
InputManager::InternalMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW scroll {} {}", xoffset, yoffset);

   BroadcastEvent(EventType::MOUSE_SCROLL, MouseScrollEvent{xoffset, yoffset});
}

void
InputManager::BroadcastEvent(EventType type, Event& event)
{
   switch (type)
   {
      case EventType::KEY: {
         for (auto listener : s_keyListeners)
         {
            listener->KeyCallback(static_cast< KeyEvent& >(event));
         }
      }
      break;

      case EventType::MOUSE_BUTTON: {
         for (auto listener : s_mouseButtonListeners)
         {
            listener->MouseButtonCallback(static_cast< MouseButtonEvent& >(event));
         }
      }
      break;

      case EventType::MOUSE_CURSOR: {
         for (auto listener : s_mouseMovementListeners)
         {
            listener->CursorPositionCallback(static_cast< CursorPositionEvent& >(event));
         }
      }
      break;

      case EventType::MOUSE_SCROLL: {
         for (auto listener : s_mouseScrollListeners)
         {
            listener->MouseScrollCallback(static_cast< MouseScrollEvent& >(event));
         }
      }
      break;

      default:
         break;
   }
}

void
InputManager::Init(GLFWwindow* mainWindow)
{
   s_windowHandle = mainWindow;
   s_logger.Init("InputManager");

   glfwSetKeyCallback(s_windowHandle, InternalKeyCallback);
   glfwSetMouseButtonCallback(s_windowHandle, InternalMouseButtonCallback);
   glfwSetCursorPosCallback(s_windowHandle, InternalCursorPositionCallback);
   glfwSetScrollCallback(s_windowHandle, InternalMouseScrollCallback);

   s_keyMap.clear();
}

void
InputManager::RegisterForKeyInput(IInputListener* listener)
{
   s_keyListeners.push_back(listener);
}

void
InputManager::RegisterForMouseButtonInput(IInputListener* listener)
{
   s_mouseButtonListeners.push_back(listener);
}

void
InputManager::RegisterForMouseMovementInput(IInputListener* listener)
{
   s_mouseMovementListeners.push_back(listener);
}

void
InputManager::RegisterForMouseScrollInput(IInputListener* listener)
{
   s_mouseScrollListeners.push_back(listener);
}

void
InputManager::PollEvents()
{
   glfwPollEvents();
}

bool
InputManager::CheckKeyPressed(int32_t keyKode)
{
   return s_keyMap[keyKode];
}

glm::vec2
InputManager::GetMousePos()
{
   return s_mousePosition;
}

} // namespace dgame