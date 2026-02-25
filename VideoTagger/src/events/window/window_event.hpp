#pragma once
#include <events/event.hpp>
#include <core/app_window.hpp>

namespace vt
{
	///@brief Base class for all window related events
	struct window_event : public event
	{
	public:
		constexpr window_event(app_window& window) : window_{ window } {}

	private:
		app_window& window_;

	public:
		///@return Reference to the window associated with this event
		constexpr app_window& window() const
		{
			return window_;
		}

		bool is_from(const app_window& window) const
		{
			return window_ == window;
		}
	};
}
