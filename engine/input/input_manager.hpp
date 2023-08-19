#pragma once

#include "input_listener.hpp"
#include "logger.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace looper {

struct Event;

class InputManager
{
 public:
   static void
   RegisterForInput(GLFWwindow* window, InputListener* listener);

   static void
   UnregisterFromInput(GLFWwindow* window, InputListener* listener);

   // @brief Polling function to check if key is currenty pressed
   // @param action Key in which user is interested
   // @return True if key is pressed, False otherwise
   static bool
   CheckKeyPressed(int32_t keyKode);

   // @brief Polling function to check if mouse button is currenty pressed
   // @param action Mouse button in which user is interested
   // @return True if button is pressed, False otherwise
   static bool
   CheckButtonPressed(int32_t button);

   // @brief Polling function to get current mouse position
   // @return Mouse position relative to window
   static glm::vec2
   GetMousePos();

   static void
   SetMousePos(const glm::vec2& position);

   // @brief Called each frame to update the input
   static void
   PollEvents();

   // @brief Initialize function which should be called once at the start of the program
   // @param mainWindow GLFW window handle for which input will be handled
   static void
   Init(GLFWwindow* mainWindow);

   // callbacks for GLFW input
   static void
   InternalKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action,
                       int32_t mods);
   static void
   InternalCharCallback(GLFWwindow* window, uint32_t key);
   static void
   InternalMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
   static void
   InternalCursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
   static void
   InternalMouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
   static void
   InternalWindowFocusCallback(GLFWwindow* window, int32_t focused);

 private:
   // NOLINTBEGIN
   static inline std::unordered_map < GLFWwindow*, std::vector< InputListener* >> listeners_ = {};

   // in future handle input from multiple windows?
   static inline GLFWwindow* windowHandle_ = nullptr;

   static inline glm::vec2 mousePosition_ = {};
   static inline std::unordered_map< int32_t, bool > mouseButtonMap_ = {};
   static inline std::unordered_map< int32_t, bool > keyMap_ = {};
   // NOLINTEND
};

} // namespace looper