#pragma once
#include "gizmo_targets_event.hpp"

namespace vt
{
	struct gizmo_set_targets_event : public gizmo_targets_event
	{
	public:
		gizmo_set_targets_event(const std::vector<utils::vec2<uint32_t>*>& targets) : gizmo_targets_event{ targets } {}
	};
}
