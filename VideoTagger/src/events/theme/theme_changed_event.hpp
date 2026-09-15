#pragma once
#include "theme_event.hpp"

namespace vt
{
	struct theme_changed_event : public theme_event
	{
		constexpr theme_changed_event(vt::theme& theme) : theme_event{ theme } {}
	};
}
