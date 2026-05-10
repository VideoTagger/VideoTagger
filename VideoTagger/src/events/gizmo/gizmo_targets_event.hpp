#pragma once
#include <vector>
#include <utils/vec.hpp>
#include "gizmo_event.hpp"
#include <core/types.hpp>

namespace vt
{
	///@brief Base class for all gizmo related events containing multiple targets
	struct gizmo_targets_event : public gizmo_event
	{
	public:
		gizmo_targets_event(video_id_t video_id, const std::vector<utils::vec2<uint32_t>*>& targets) : video_id_{ video_id }, targets_ { targets } {}

	private:
		std::vector<utils::vec2<uint32_t>*> targets_;
		video_id_t video_id_;

	public:
		[[nodiscard]] const std::vector<utils::vec2<uint32_t>*>& targets() const
		{
			return targets_;
		}

		video_id_t video_id() const
		{
			return video_id_;
		}
	};
}
