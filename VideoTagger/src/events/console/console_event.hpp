#pragma once
#include <events/event.hpp>
#include <ui/windows/console.hpp>

namespace vt
{
	///@brief Base class for all console related events
	struct console_event : public event
	{
	public:
		constexpr console_event(ui::windows::console& console) : console_{ &console } {}

	private:
		ui::windows::console* console_;

	public:
		///@return Reference to the console associated with this event
		constexpr ui::windows::console& console() const
		{
			return *console_;
		}
	};
}
