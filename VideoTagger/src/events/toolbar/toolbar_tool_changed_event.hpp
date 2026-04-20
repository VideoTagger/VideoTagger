#pragma once
#include <events/toolbar/toolbar_tool_event.hpp>

namespace vt
{
	struct toolbar_tool_changed_event : public toolbar_tool_event
	{
		toolbar_tool_changed_event(const ui::toolbar_tool& tool) : toolbar_tool_event{ tool } {}
	};
}
