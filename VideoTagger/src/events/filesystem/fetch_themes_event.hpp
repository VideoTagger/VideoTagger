#pragma once
#include <events/event.hpp>

namespace vt
{
	struct fetch_themes_event : public event
	{
		fetch_themes_event() = default;
	};
}
