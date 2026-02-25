#pragma once
#include "window_event.hpp"

namespace vt
{
	struct window_close_event : public window_event
	{
		constexpr window_close_event(app_window& window) : window_event{ window } {}
	};
}
