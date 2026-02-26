#pragma once
#include "ui_window_event.hpp"

namespace vt
{
	struct ui_window_unregister_event : public ui_window_event
	{
		constexpr ui_window_unregister_event(ui::window& window) : ui_window_event{ window } {}
	};
}
