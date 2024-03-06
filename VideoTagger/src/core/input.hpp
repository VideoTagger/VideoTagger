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
		std::function<void(void)> keyboard_shortcut_function;
	};
	void input_function(SDL_Event &event, app_context &ctx);
}
