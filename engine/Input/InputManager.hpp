#pragma once

#include "IInputListener.hpp"
#include "Logger.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

class InputManager
{
 public:
   // @brief Used by IInputListener object to register for key callback
   // @param listener listener object interested in
   // @param state state of the action in which user is interested
   void
   RegisterForKeyInput(IInputListener* listener);

   // @brief Polling function to check if key is currenty pressed
   // @param action Key in which user is interested
   // @return True if key is pressed, False otherwise
   bool
   CheckKeyPressed(int keyKode);

   // @brief Polling function to check if key mapped to given action is currenty pressed
   // @param action Action in which user is interested
   // @return True if action is active, False otherwise
   // bool
   // CheckAction(std::string_view action);

   // @brief Read action->key config file and store results in m_actionKeyMap
   // @param configFile File which contains action->key map
   /*void
   SetUpKeyMap(const std::string& configFile);*/

   // @brief Called each frame to update the input and notify all interested components
   void
   HandleInput();

   void
   Init(GLFWwindow* mainWindow);

   // callbacks for GLFW input
   static void
   InternalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
   static void
   InternalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
   static void
   InternalCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

 private:
   std::vector< IInputListener* > m_inputListeners;

   // in future handle input from multiple windows?
   GLFWwindow* m_windowHandle;

   static inline glm::vec2 m_mousePosition = {};
   static inline std::unordered_map< int, bool > m_keyMap = {};
   static inline Logger m_logger = Logger("InputManager");
};
