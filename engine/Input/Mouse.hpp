#pragma once

#include <SDL_mouse.h>

namespace noc::input
{

enum class NOC_MOUSEBUTTON : uint32_t
{
	LEFT = SDL_BUTTON_LEFT,
	MIDDLE = SDL_BUTTON_MIDDLE,
	RIGTH = SDL_BUTTON_RIGHT,
	X1 = SDL_BUTTON_X1,
	X2 = SDL_BUTTON_X2,
};

}   // namespace noc::input
