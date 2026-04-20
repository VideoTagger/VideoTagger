#pragma once
#include <events/toolbar/toolbar_event.hpp>
#include <ui/toolbar_tool.hpp>

namespace vt
{
	///@brief Base class for all toolbar tool related events
	struct toolbar_tool_event : public toolbar_event
	{
	public:
		constexpr toolbar_tool_event(const ui::toolbar_tool& tool) : tool_{ &tool } {}

	private:
		const ui::toolbar_tool* tool_;

	public:
		///@return Reference to the toolbar tool associated with this event
		constexpr const ui::toolbar_tool& tool() const
		{
			return *tool_;
		}
	};
}
