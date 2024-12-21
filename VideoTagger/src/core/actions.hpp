#pragma once
#include <vector>
#include <memory>
#include "actions/keybind_action.hpp"
#include "actions/builtin_action.hpp"
#include "actions/no_action.hpp"
#include "actions/timeline_actions.hpp"
#include "actions/toggle_window_action.hpp"
#include "actions/player_actions.hpp"
#include "actions/run_script_action.hpp"

namespace vt
{
	extern std::vector<std::shared_ptr<keybind_action>> get_all_keybind_actions();

	inline void from_json(const nlohmann::ordered_json& json, std::shared_ptr<keybind_action>& action)
	{
		if (!json.contains("name"))
		{
			action = nullptr;
			return;
		}

		const auto& json_name = json.at("name");
		if (json_name == no_action::action_name) action = std::make_shared<no_action>();
		else if (json_name == timestamp_action::action_name) action = std::make_shared<timestamp_action>();
		else if (json_name == segment_action::action_name) action = std::make_shared<segment_action>();
		else if (json_name == run_script_action::action_name) action = std::make_shared<run_script_action>();

		if (action != nullptr and json.contains("data"))
		{
			action->from_json(json.at("data"));
		}
	}
}
