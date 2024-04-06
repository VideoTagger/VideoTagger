#include "pch.hpp"
#include "builtin_action.hpp"

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

	void builtin_action::render_properties()
	{
		
	}
}
