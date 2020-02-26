#pragma once

#include "KeyCodes.hpp"
#include "IInputListener.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;

class InputManager
{
public:

	//union InputEvent {
	//	NOC_KEY         key;
	//	NOC_MOUSEBUTTON button;
	//};

	//InputEvent operator()(NOC_KEY key)
	//{
	//	InputEvent e;
	//	e.key = key;
	//	return e;
	//}

public:
	// @brief Used by UserControllable object to register for certain action
	// @param sactionName string represented action
	// @param state state of the action in which user is interested
	// void
	// RegisterForActionEvent(UserControllable* object, std::string_view actionName, NOC_KEY_STATE state);

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
	static void InternalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void InternalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void InternalMousePositionCallback(GLFWwindow* window, double xpos, double ypos);

private:
	/*void
	SetInputEventPressed(InputEvent inputPressed);

	void
	SetInputEventReleased(InputEvent inputReleased);


	void
	SetMousePosition(const glm::vec2& mousePos);

	std::unordered_map<std::string, InputEvent> m_actionKeyMap;
	std::unordered_map<InputEvent, bool>        m_inputEventState;*/
	std::vector<IInputListener*> m_inputListeners;

	glm::vec2 m_mousePosition;

	// in future handle input from multiple windows?
	GLFWwindow* m_windowHandle;
};
