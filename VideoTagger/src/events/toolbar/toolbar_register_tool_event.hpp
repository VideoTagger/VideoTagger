#pragma once
#include <events/toolbar/toolbar_tool_event.hpp>

namespace vt
{
	struct toolbar_register_tool_event : public toolbar_tool_event
	{
		toolbar_register_tool_event(const ui::toolbar_group& group, const ui::toolbar_group_entry& group_entry, const ui::toolbar_tool& tool) : toolbar_tool_event{ group, group_entry, tool } {}
	};
}
