#include "pch.hpp"
#include "toggle_window_action.hpp"
#include <core/app_context.hpp>

namespace vt
{
	toggle_window_action::toggle_window_action(const char* settings_name, bool& value) : keybind_action("Toggle Window"), settings_name{ settings_name }, value { value } {}

	void toggle_window_action::invoke() const
	{
		value = !value;
		ctx_.settings[settings_name] = value;
	}

	void toggle_window_action::render_properties() {}
}
