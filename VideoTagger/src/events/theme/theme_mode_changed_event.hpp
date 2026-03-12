#pragma once
#include "theme_event.hpp"

namespace vt
{
	struct theme_mode_changed_event : public theme_event
	{
	public:
		constexpr theme_mode_changed_event(vt::theme& theme, bool is_dark) : theme_event{ theme }, is_dark_{ is_dark } {}

	private:
		bool is_dark_;

	public:
		///@return True if the theme mode is dark, false if it is light
		constexpr bool is_dark() const
		{
			return is_dark_;
		}
	};
}
