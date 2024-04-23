#include "pch.hpp"
#include "no_action.hpp"

namespace vt
{
	no_action::no_action() : keybind_action(action_name) {}
	void no_action::invoke() const {}
	void no_action::to_json(nlohmann::ordered_json& json) const {}
	void no_action::from_json(const nlohmann::ordered_json& json) {}
	void no_action::render_properties() {}
}
