#pragma once
#include <core/types.hpp>
#include "video_group_event.hpp"

namespace vt
{
	///@brief Base class for all video related events, connected with a video group
	struct video_group_video_event : public video_group_event
	{
	public:
		video_group_video_event(video_group_id_t group_id, video_id_t video_id) : video_group_event{ group_id }, video_id_{ video_id } {}

	private:
		video_id_t video_id_;

	public:
		video_id_t video_id() const
		{
			return video_id_;
		}
	};
}
