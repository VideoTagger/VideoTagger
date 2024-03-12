#include "input.hpp"

namespace vt
{
	keybind_modifiers::keybind_modifiers(bool ctrl, bool shift, bool alt) : ctrl{ ctrl }, shift{ shift }, alt{ alt } {}
	keybind::keybind(int key_code, const std::function<void()>& action) : key_code{ key_code }, modifiers{}, action{ action } {}
	keybind::keybind(int key_code, keybind_modifiers modifiers, const std::function<void()>& action) : key_code{ key_code }, modifiers{ modifiers }, action{ action } {}

	void process_inputs(SDL_Event& event, const std::unordered_map<std::string, vt::keybind>& keybinds)
	{
		if (event.type != SDL_KEYDOWN) return;

		for (const auto& [name, keybind] : keybinds)
		{
			if (keybind.key_code < 0) continue;

			bool condition = keybind.key_code == event.key.keysym.sym;
			condition &= keybind.modifiers.alt == ((event.key.keysym.mod & KMOD_ALT) != 0);
			condition &= keybind.modifiers.ctrl == ((event.key.keysym.mod & KMOD_CTRL) != 0);
			condition &= keybind.modifiers.shift == ((event.key.keysym.mod & KMOD_SHIFT) != 0);

			if (condition and keybind.action != nullptr)
			{
				std::invoke(keybind.action);
				return;
			}
		}
	}
}
