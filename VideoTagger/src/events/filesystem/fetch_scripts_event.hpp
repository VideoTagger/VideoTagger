#pragma once
#include <events/event.hpp>

namespace vt
{
	struct fetch_scripts_event : public event
	{
		fetch_scripts_event() = default;
	};
}
