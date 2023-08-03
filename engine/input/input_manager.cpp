#include "input_manager.hpp"
#include "event.hpp"
#include "types.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <functional>
#include <iostream>

namespace looper {

template < auto func >
void
BroadcastEvent(const auto& listeners, auto& event)
{
   for (auto* listener : listeners)
   {
      std::invoke(func, listener, event);

      if (event.handled_)
      {
         break;
      }
   }
}

void
InputManager::InternalKeyCallback(GLFWwindow* /*window*/, int32_t key, int32_t scancode,
                                  int32_t action, int32_t mods)
{
   Logger::Trace("GLFW key {} {} scan code - {}", action, key, scancode);

   keyMap_[key] = action;
   auto event = KeyEvent{key, scancode, action, mods};

   BroadcastEvent< &InputListener::KeyCallback >(listeners_, event);
}

void
InputManager::InternalCharCallback(GLFWwindow* /*window*/, uint32_t key)
{
   Logger::Trace("GLFW char {}", key);

   auto event = CharEvent{key};

   BroadcastEvent< &InputListener::CharCallback >(listeners_, event);
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow* /*window*/, int32_t button, int32_t action,
                                          int32_t mods)
{
   Logger::Trace("GLFW mouse button {} {} {}", button, action, mods);
   mouseButtonMap_[button] = action;

   auto event = MouseButtonEvent{button, action, mods};
   BroadcastEvent< &InputListener::MouseButtonCallback >(listeners_, event);
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow* /*window*/, double xPos, double yPos)
{
   Logger::Trace("GLFW cursor pos {} {}", xPos, yPos);

   mousePosition_ = glm::vec2(xPos, yPos);
   auto event = CursorPositionEvent{xPos, yPos};

   BroadcastEvent< &InputListener::CursorPositionCallback >(listeners_, event);
}

void
InputManager::InternalMouseScrollCallback(GLFWwindow* /*window*/, double xOffset, double yOffset)
{
   Logger::Trace("GLFW scroll {} {}", xOffset, yOffset);

   auto event = MouseScrollEvent{xOffset, yOffset};

   BroadcastEvent< &InputListener::MouseScrollCallback >(listeners_, event);
}

void
InputManager::InternalWindowFocusCallback(GLFWwindow* /*window*/, int32_t focused)
{
   Logger::Trace("GLFW window focus {}", focused);

   auto event = WindowFocusEvent{focused};

   BroadcastEvent< &InputListener::WindowFocusCallback >(listeners_, event);
}

void
InputManager::Init(GLFWwindow* mainWindow)
{
   windowHandle_ = mainWindow;

   glfwSetKeyCallback(windowHandle_, InternalKeyCallback);
   glfwSetCharCallback(windowHandle_, InternalCharCallback);
   glfwSetMouseButtonCallback(windowHandle_, InternalMouseButtonCallback);
   glfwSetCursorPosCallback(windowHandle_, InternalCursorPositionCallback);
   glfwSetScrollCallback(windowHandle_, InternalMouseScrollCallback);
   glfwSetWindowFocusCallback(windowHandle_, InternalWindowFocusCallback);
   keyMap_.clear();
}

void
InputManager::RegisterForInput(InputListener* listener)
{
   listeners_.push_back(listener);
}

void
InputManager::UnregisterFromInput(InputListener* listener)
{
   auto foundListener = stl::find_if(listeners_, [listener](const auto& registeredListener) {
      return listener == registeredListener;
   });
   if (foundListener != listeners_.end())
   {
      listeners_.erase(foundListener);
   }
   else
   {
      Logger::Debug("Listener not found!");
   }
}

void
InputManager::PollEvents()
{
   glfwPollEvents();
}

bool
InputManager::CheckKeyPressed(int32_t keyKode)
{
   return keyMap_[keyKode];
}

bool
InputManager::CheckButtonPressed(int32_t button)
{
   return mouseButtonMap_[button];
}

glm::vec2
InputManager::GetMousePos()
{
   return mousePosition_;
}

void
InputManager::SetMousePos(const glm::vec2& position)
{
   glfwSetCursorPos(windowHandle_, static_cast< double >(position.x),
                    static_cast< double >(position.y));
}

} // namespace looper
