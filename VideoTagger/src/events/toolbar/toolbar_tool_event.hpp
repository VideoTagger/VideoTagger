#pragma once
#include <ui/toolbar/toolbar_tool.hpp>
#include <ui/toolbar/toolbar_group.hpp>
#include <events/toolbar/toolbar_event.hpp>

namespace vt
{
	///@brief Base class for all toolbar tool related events
	struct toolbar_tool_event : public toolbar_event
	{
	public:
		constexpr toolbar_tool_event(ui::toolbar_group& group, ui::toolbar_group_entry& group_entry, ui::toolbar_tool& tool) : group_{ &group }, group_entry_{ &group_entry }, tool_ { &tool } {}

	private:
		ui::toolbar_group* group_;
		ui::toolbar_group_entry* group_entry_;
		ui::toolbar_tool* tool_;

	public:
		///@return Reference to the toolbar group associated with this event
		constexpr ui::toolbar_group& group() const
		{
			return *group_;
		}

		///@return Reference to the toolbar group entry associated with this event
		constexpr ui::toolbar_group_entry& group_entry() const
		{
			return *group_entry_;
		}

		///@return Reference to the toolbar tool associated with this event
		constexpr ui::toolbar_tool& tool() const
		{
			return *tool_;
		}
	};
}
