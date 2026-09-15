#pragma once
#include <events/event.hpp>
#include <system/system_window.hpp>

namespace vt
{
	///@brief Base class for all system window related events
	struct system_window_event : public event
	{
	public:
		constexpr system_window_event(system_window& window) : window_{ window } {}

	private:
		system_window& window_;

	public:
		///@return Reference to the window associated with this event
		constexpr system_window& window() const
		{
			return window_;
		}

		bool is_from(const system_window& window) const
		{
			return window_ == window;
		}
	};
}
