#pragma once
#include <events/event.hpp>

namespace vt
{
	///@brief Base class for all project selector related events
	struct project_selector_event : public event
	{
		project_selector_event() = default;
	};
}
