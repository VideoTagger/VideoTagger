#include <iostream>
#include <SDL.h>

namespace vt::utils::input
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
	};

	keybind input_function(SDL_Event& event)
	{
		keybind myKeybind = {};
		myKeybind.key_code = event.key.keysym.sym;
		myKeybind.modifiers.ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
		myKeybind.modifiers.shift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
		myKeybind.modifiers.alt = (event.key.keysym.mod & KMOD_ALT) != 0;

		/*
		std::cout << "Key pressed: " << myKeybind.key_code << std::endl;

		if (myKeybind.modifiers.ctrl) {
			std::cout << "Ctrl is pressed" << std::endl;
		}
		if (myKeybind.modifiers.shift) {
			std::cout << "Shift is pressed" << std::endl;
		}
		if (myKeybind.modifiers.alt) {
			std::cout << "Alt is pressed" << std::endl;
		}
		*/
		
		return myKeybind;
	}
}
