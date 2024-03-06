#include "input.hpp"
#include <SDL.h>
#include <core/app_context.hpp>
#include <iostream>

namespace vt
{
	
	bool is_ctrl_pressed(SDL_Event& event);
	bool is_alt_pressed(SDL_Event& event);
	bool is_shift_pressed(SDL_Event& event);

	void input_function(SDL_Event& event, app_context& ctx)
	{
		for (auto& [name, keybind] : ctx.keybinds)
		{
			if (keybind.modifiers.alt = is_alt_pressed(event));
			if (keybind.modifiers.ctrl = is_ctrl_pressed(event));
			if (keybind.modifiers.shift = is_shift_pressed(event));
			if (keybind.key_code = event.key.keysym.sym); {
				std::cout << "DOING \n" << name; //Tymczasowe na testy
				keybind.keyboard_shortcut_function();
			}
			
		}
	
	}
	bool is_ctrl_pressed(SDL_Event& event)
	{
		return ((event.key.keysym.mod & KMOD_CTRL) != 0);
	}
	bool is_shift_pressed(SDL_Event& event)
	{
		return ((event.key.keysym.mod & KMOD_SHIFT) != 0);
	}
	bool is_alt_pressed(SDL_Event& event)
	{
		return ((event.key.keysym.mod & KMOD_ALT) != 0);
	}
}
