#pragma once
#include "project_selector_event.hpp"

namespace vt
{
	struct project_list_changed_event : public project_selector_event
	{
	public:
		project_list_changed_event() {}
	};
}
