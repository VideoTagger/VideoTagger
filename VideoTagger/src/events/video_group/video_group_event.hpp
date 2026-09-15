#pragma once
#include <core/types.hpp>
#include <events/event.hpp>

namespace vt
{
	///@brief Base class for all video group related events
	struct video_group_event : public event
	{
	public:
		video_group_event(video_group_id_t group_id) : group_id_(group_id) {}

	private:
		video_group_id_t group_id_;

	public:
		video_group_id_t group_id() const
		{
			return group_id_;
		}
	};
}
