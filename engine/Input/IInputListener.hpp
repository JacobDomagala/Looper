#pragma once

#include <SDL_events.h>
#include <string_view>

namespace noc::input
{
class Input;

class UserControllable
{
public:
	// @brief Callback function called each frame
	// @param key key code
	// @param state NOC_KEY_STATE::PRESSED or NOC_KEY_STATE::RELEASED
	/*virtual void
	HandleInput(NOC_KEY key, NOC_KEY_STATE state)
	   = 0;*/

	// @brief Callback function called each frame
	// @param actionName name of the action that changed
	// @param state NOC_KEY_STATE::PRESSED or NOC_KEY_STATE::RELEASED
	/*virtual void
	HandleAction(std::string_view actionName, NOC_KEY_STATE state)
	   = 0;*/

	virtual void
	SetUpInput(Input& input)
	{
	}
};

}   // namespace noc::input
