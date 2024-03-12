#pragma once
#include <string>
#include <functional>
#include <SDL.h>

namespace vt
{
	struct keybind
	{
		int key_code{};
		union modifiers
		{
			uint8_t flags{};
			struct
			{
				bool ctrl : 1;
				bool shift : 1;
				bool alt : 1;
			};
		} modifiers;
		std::function<void(void)> action;
	};

	extern void process_inputs(SDL_Event& event, const std::unordered_map<std::string, vt::keybind>& keybinds);
}
