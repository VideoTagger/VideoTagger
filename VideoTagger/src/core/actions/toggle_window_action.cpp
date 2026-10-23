#include "pch.hpp"
#include "toggle_window_action.hpp"
#include <core/app_context.hpp>

namespace vt
{
	toggle_window_action::toggle_window_action(ui::window& window) : keybind_action(action_name), window_{ window } {}

	void toggle_window_action::invoke() const
	{
		window_.set_opened(!window_.is_open());
	}

	void toggle_window_action::to_json(nlohmann::ordered_json& json) const
	{
		debug::error("Toggle window actions cannot be serialized");
	}

	void toggle_window_action::from_json(const nlohmann::ordered_json& json)
	{
		debug::error("Toggle window actions cannot be deserialized");
	}

	void toggle_window_action::render_properties() {}
}
