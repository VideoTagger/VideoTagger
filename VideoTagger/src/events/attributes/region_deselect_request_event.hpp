#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_deselect_request_event : public event
	{
	public:
		region_deselect_request_event() = default;
	};
}
