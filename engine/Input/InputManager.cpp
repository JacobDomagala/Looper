#include "InputManager.hpp"
#include "Event.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

namespace dgame {

void
InputManager::InternalKeyCallback(GLFWwindow*, int32_t key, int32_t scancode, int32_t action,
                                  int32_t mods)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW key {} {} scan code - {}", action, key, scancode);

   s_keyMap[key] = action;

   BroadcastEvent(KeyEvent{key, scancode, action, mods});
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow*, int32_t button, int32_t action, int32_t mods)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW mouse button {} {} {}", button, action, mods);

   BroadcastEvent(MouseButtonEvent{button, action, mods});
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow*, double x, double y)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW cursor pos {} {}", x, y);

   s_mousePosition = glm::vec2(x, y);

   BroadcastEvent(CursorPositionEvent{x, y});
}

void
InputManager::InternalMouseScrollCallback(GLFWwindow*, double xoffset, double yoffset)
{
   s_logger.Log(Logger::TYPE::TRACE, "GLFW scroll {} {}", xoffset, yoffset);

   BroadcastEvent(MouseScrollEvent{xoffset, yoffset});
}

void
InputManager::BroadcastEvent(const Event& event)
{
   switch (event.m_type)
   {
      case Event::EventType::KEY: {
         for (auto listener : s_keyListeners)
         {
            listener->KeyCallback(static_cast< const KeyEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_BUTTON: {
         for (auto listener : s_mouseButtonListeners)
         {
            listener->MouseButtonCallback(static_cast< const MouseButtonEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_CURSOR: {
         for (auto listener : s_mouseMovementListeners)
         {
            listener->CursorPositionCallback(static_cast< const CursorPositionEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_SCROLL: {
         for (auto listener : s_mouseScrollListeners)
         {
            listener->MouseScrollCallback(static_cast< const MouseScrollEvent& >(event));
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
InputManager::RegisterForKeyInput(InputListener* listener)
{
   s_keyListeners.push_back(listener);
}

void
InputManager::RegisterForMouseButtonInput(InputListener* listener)
{
   s_mouseButtonListeners.push_back(listener);
}

void
InputManager::RegisterForMouseMovementInput(InputListener* listener)
{
   s_mouseMovementListeners.push_back(listener);
}

void
InputManager::RegisterForMouseScrollInput(InputListener* listener)
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

void
InputManager::SetMousePos(const glm::vec2& position)
{
   glfwSetCursorPos(s_windowHandle, static_cast< double >(position.x),
                    static_cast< double >(position.y));
}

} // namespace dgame
