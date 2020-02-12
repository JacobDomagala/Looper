#pragma once

#include "KeyCodes.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace noc::input
{
class UserControllable;

class Input
{
public:

	union InputEvent {
		NOC_KEY         key;
		NOC_MOUSEBUTTON button;
	};

	InputEvent operator()(NOC_KEY key)
	{
		InputEvent e;
		e.key = key;
		return e;
	}

public:
	// @brief Used by UserControllable object to register for certain action
	// @param sactionName string represented action
	// @param state state of the action in which user is interested
	void
	RegisterForActionEvent(UserControllable* object, std::string_view actionName, NOC_KEY_STATE state);

	// @brief Polling function to check if key is currenty pressed
	// @param action Key in which user is interested
	// @return True if key is pressed, False otherwise
	bool
	CheckKeyPressed(NOC_KEY keyKode);

	// @brief Polling function to check if key mapped to given action is currenty pressed
	// @param action Action in which user is interested
	// @return True if action is active, False otherwise
	bool
	CheckAction(std::string_view action);

	// @brief Read action->key config file and store results in m_actionKeyMap
	// @param configFile File which contains action->key map
	void
	SetUpKeyMap(std::string_view configFile);

	// @brief Called each frame to update the input and notify all interested components
	void
	HandleInput();

private:
	void
	SetInputEventPressed(InputEvent inputPressed);

	void
	SetInputEventReleased(InputEvent inputReleased);

	// TODO CHANGE TO VEC2
	void
	SetMousePosition(uint32_t x, uint32_t y);

	std::unordered_map<std::string, InputEvent> m_actionKeyMap;
	std::unordered_map<InputEvent, bool>        m_inputEventState;

	// TODO: CHANGE TO VEC2
	uint32_t m_mouseX;
	uint32_t m_mouseY;
};

}   // namespace noc::input
