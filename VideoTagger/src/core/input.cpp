#include "input.hpp"

namespace vt
{
	void process_inputs(SDL_Event& event, const std::unordered_map<std::string, vt::keybind>& keybinds)
	{
		if (event.type != SDL_KEYDOWN) return;

		for (const auto& [name, keybind] : keybinds)
		{
			bool condition = keybind.key_code == event.key.keysym.sym;
			condition &= keybind.modifiers.alt and (event.key.keysym.mod & KMOD_ALT) != 0;
			condition &= keybind.modifiers.ctrl and (event.key.keysym.mod & KMOD_CTRL) != 0;
			condition &= keybind.modifiers.shift and (event.key.keysym.mod & KMOD_SHIFT) != 0;

			if (condition and keybind.action != nullptr)
			{
				std::invoke(keybind.action);
				return;
			}
		}
	}
}
