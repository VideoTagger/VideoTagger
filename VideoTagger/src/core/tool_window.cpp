#include <pch.hpp>
#include "tool_window.hpp"

namespace vt
{
	tool_window::tool_window(const app_window_config& cfg) : app_window{ cfg } {}
    void tool_window::draw()
    {

    }

	void tool_window::handle_event(const SDL_Event& event)
	{
		app_window::handle_event(event);

		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				if (event.window.windowID != 1) return;

				switch (event.window.type)
				{
					case SDL_WINDOWEVENT_CLOSE:
					{
						show(false);
					}
					break;
				}
			}
			break;
		}
	}
}
