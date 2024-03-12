#pragma once
#include <string>
#include <functional>
#include <SDL.h>

namespace vt
{
	union keybind_modifiers
	{
	public:
		keybind_modifiers(bool ctrl = false, bool shift = false, bool alt = false);

	private:
		uint8_t flags{};
	public:
		struct
		{
			bool ctrl : 1;
			bool shift : 1;
			bool alt : 1;
		};
	};

	struct keybind
	{
		keybind() = default;
		keybind(int key_code, const std::function<void()>& action);
		keybind(int key_code, keybind_modifiers modifiers, const std::function<void()>& action);

		keybind_modifiers modifiers;
		std::function<void()> action;
		int key_code = -1;
	};

	extern void process_inputs(SDL_Event& event, const std::unordered_map<std::string, vt::keybind>& keybinds);
}
