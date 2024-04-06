#include "pch.hpp"
#include "actions.hpp"

namespace vt
{
	std::vector<std::shared_ptr<keybind_action>> get_all_keybind_actions()
	{
		std::vector<std::shared_ptr<keybind_action>> actions;
		actions.push_back(std::make_shared<no_action>());
		actions.push_back(std::make_shared<add_timestamp_action>());
		actions.push_back(std::make_shared<segment_action>());
		return actions;
	}
}
