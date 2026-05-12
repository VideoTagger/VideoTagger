#pragma once
#include "gizmo_targets_event.hpp"

namespace vt
{
	struct gizmo_set_targets_event : public gizmo_targets_event
	{
	public:
		gizmo_set_targets_event(video_id_t video_id, const std::vector<utils::vec2<int>*>& targets = {}) : gizmo_targets_event{ video_id, targets } {}
	};
}
