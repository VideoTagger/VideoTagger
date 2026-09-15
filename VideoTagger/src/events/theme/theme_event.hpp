#pragma once
#include <events/event.hpp>
#include <core/theme.hpp>

namespace vt
{
	///@brief Base class for all theme related events
	struct theme_event : public event
	{
	public:
		constexpr theme_event(theme& theme) : theme_{ &theme } {}

	private:
		theme* theme_;

	public:
		///@return Reference to the theme associated with this event
		constexpr theme& theme() const
		{
			return *theme_;
		}
	};
}
