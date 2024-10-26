#pragma once
#include <SDL.h>
#include "app_window.hpp"

namespace vt
{
	class tool_window : public app_window
	{
	public:
		tool_window(const app_window_config& cfg);

	public:
		virtual void draw() override;
		virtual void handle_event(const SDL_Event& event) override;
	};
}
