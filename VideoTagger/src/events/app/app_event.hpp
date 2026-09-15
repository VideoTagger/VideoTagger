#pragma once
#include <events/event.hpp>

namespace vt
{
	///@brief Base class for all app related events
	struct app_event : public event
	{
		app_event() = default;
	};
}
