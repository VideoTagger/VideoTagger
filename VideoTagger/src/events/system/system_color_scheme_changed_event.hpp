#pragma once
#include <events/event.hpp>
#include <system/system_window.hpp>

namespace vt
{
	struct system_color_scheme_changed_event : public event
	{
	public:
		constexpr system_color_scheme_changed_event(bool is_dark) : is_dark_{ is_dark } {}

	private:
		bool is_dark_;

	public:
		///@return True if the new mode is dark, false if otherwise
		constexpr bool is_dark() const
		{
			return is_dark_;
		}
	};
}
