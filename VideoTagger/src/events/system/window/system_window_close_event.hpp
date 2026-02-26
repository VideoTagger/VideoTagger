#pragma once
#include "system_window_event.hpp"

namespace vt
{
	struct system_window_close_event : public system_window_event
	{
		constexpr system_window_close_event(system_window& window) : system_window_event{ window } {}
	};
}
