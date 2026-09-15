#pragma once
#include <string>

namespace vt::ui
{
	struct toolbar_tool_specification
	{
		toolbar_tool_specification() = default;
		toolbar_tool_specification(const std::string& id, const std::string& icon = "", const std::string& tooltip = "", bool is_persistent = false) :
			id{ id }, icon{ icon }, tooltip{ tooltip }, is_persistent{ is_persistent } {}

		std::string id;
		std::string icon;
		std::string tooltip;
		///@brief If true, the tool won't be deleted during the tool re-registration phase.
		bool is_persistent = false;
		bool should_always_display_body = false;
	};
}
