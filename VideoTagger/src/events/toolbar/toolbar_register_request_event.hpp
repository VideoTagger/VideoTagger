#pragma once
#include <events/toolbar/toolbar_event.hpp>

namespace vt
{
	///@brief Event dispatched by a toolbar to request all tools to be re-registered.
	struct toolbar_register_request_event : public toolbar_event
	{
		toolbar_register_request_event() {}
	};
}
