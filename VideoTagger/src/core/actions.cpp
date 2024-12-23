#include "pch.hpp"
#include "actions.hpp"
#include "actions/run_script_action.hpp"

namespace vt
{
	std::vector<std::shared_ptr<keybind_action>> get_all_keybind_actions()
	{
		std::vector<std::shared_ptr<keybind_action>> actions;
		actions.push_back(std::make_shared<no_action>());
		actions.push_back(std::make_shared<timestamp_action>());
		actions.push_back(std::make_shared<segment_action>());
		actions.push_back(std::make_shared<run_script_action>());

		return actions;
	}
}
