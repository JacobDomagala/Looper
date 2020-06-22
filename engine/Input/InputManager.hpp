#pragma once

#include "IInputListener.hpp"
#include "Logger.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace dgame {

struct Event;

class InputManager
{
 public:
   enum class EventType
   {
      KEY,
      MOUSE_BUTTON,
      MOUSE_CURSOR,
      MOUSE_SCROLL
   };

 public:
   // Functions used by IInputListener to register for input callbacks
   static void
   RegisterForKeyInput(IInputListener* listener);

   static void
   RegisterForMouseButtonInput(IInputListener* listener);

   static void
   RegisterForMouseMovementInput(IInputListener* listener);

   static void
   RegisterForMouseScrollInput(IInputListener* listener);

   // @brief Polling function to check if key is currenty pressed
   // @param action Key in which user is interested
   // @return True if key is pressed, False otherwise
   static bool
   CheckKeyPressed(int32_t keyKode);

   // @brief Polling function to get current mouse position
   // @return Mouse position relative to window
   static glm::vec2
   GetMousePos();

   // @brief Called each frame to update the input
   static void
   PollEvents();

   // @brief Initialize function which should be called once at the start of the program
   // @param mainWindow GLFW window handle for which input will be handled
   static void
   Init(GLFWwindow* mainWindow);

   // callbacks for GLFW input
   static void
   InternalKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
   static void
   InternalMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
   static void
   InternalCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
   static void
   InternalMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

 private:
   static void
   BroadcastEvent(EventType type, Event& event);

 private:
   static inline std::vector< IInputListener* > s_keyListeners = {};
   static inline std::vector< IInputListener* > s_mouseButtonListeners = {};
   static inline std::vector< IInputListener* > s_mouseMovementListeners = {};
   static inline std::vector< IInputListener* > s_mouseScrollListeners = {};

   // in future handle input from multiple windows?
   static inline GLFWwindow* s_windowHandle = nullptr;

   static inline glm::vec2 s_mousePosition = {};
   static inline std::unordered_map< int32_t, bool > s_keyMap = {};
   static inline Logger s_logger = Logger("InputManager");
};

} // namespace dgame