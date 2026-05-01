#pragma once
#include "video_group_video_event.hpp"

namespace vt
{
	struct video_group_remove_video_event : public video_group_video_event
	{
	public:
		video_group_remove_video_event(video_group_id_t group_id, video_id_t video_id) : video_group_video_event{ group_id, video_id } {}
	};
}
