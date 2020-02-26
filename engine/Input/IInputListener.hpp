#pragma once

#include <string_view>

class IInputListener
{

public:
	// @brief Callback function called each frame
	// @param key key code
	// @param scancode
	// @param action
	// @param mods
	virtual
	void KeyCallback(int key, int scancode, int action, int mods) = 0;
};

