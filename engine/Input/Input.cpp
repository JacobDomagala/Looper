#include "Input.hpp"
#include <SDL_events.h>
#include <functional>
#include <iostream>
#include <KeyCodesUtils.hpp>
#include "Mouse.hpp"

namespace noc::input
{

void
Input::HandleInput()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_MOUSEMOTION:
				SetMousePosition(event.motion.x, event.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				SetInputEventPressed(static_cast<NOC_MOUSEBUTTON>(event.button.button));
				break;
			case SDL_KEYDOWN: 
				SetInputEventPressed(event.key.keysym.sym);
				break;

			case SDL_MOUSEBUTTONUP:
				//SetInputEventReleased();
				break;
			case SDL_KEYUP:
				//SetInputEventReleased();
				break;

			case SDL_MOUSEWHEEL:
				break;

			// OTHER EVENTS
			default:
				break;
		}
	}
}

}   // namespace noc::input
