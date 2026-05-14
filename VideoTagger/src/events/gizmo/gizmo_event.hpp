#pragma once
#include <events/event.hpp>

namespace vt
{
	///@brief Base class for all gizmo related events
	struct gizmo_event : public event
	{
		gizmo_event() = default;
	};
}
