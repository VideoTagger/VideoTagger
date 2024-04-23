#include "pch.hpp"
#include "builtin_action.hpp"
#include <core/debug.hpp>

namespace vt
{
	builtin_action::builtin_action(const std::function<void()>& action, const std::string& name) : keybind_action(name), action_{ action } {}

	void builtin_action::invoke() const
	{
		if (action_ != nullptr)
		{
			std::invoke(action_);
		}
	}

	void builtin_action::to_json(nlohmann::ordered_json& json) const
	{
		debug::error("Builtin actions cannot be serialized");
	}

	void builtin_action::from_json(const nlohmann::ordered_json& json)
	{
		debug::error("Builtin actions cannot be deserialized");
	}

	void builtin_action::render_properties() {}
}
