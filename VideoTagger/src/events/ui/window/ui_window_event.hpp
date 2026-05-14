#pragma once
#include <events/event.hpp>
#include <ui/window.hpp>

namespace vt
{
	///@brief Base class for all window related events
	struct ui_window_event : public event
	{
	public:
		constexpr ui_window_event(ui::window& window) : window_{ &window } {}

	private:
		ui::window* window_;

	public:
		///@return Reference to the window associated with this event
		constexpr ui::window& window() const
		{
			return *window_;
		}

		bool is_from(const ui::window& window) const
		{
			return *window_ == window;
		}
	};
}
