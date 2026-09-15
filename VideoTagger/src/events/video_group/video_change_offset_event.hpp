#pragma once
#include "video_group_video_event.hpp"

namespace vt
{
	struct video_change_offset_event : public video_group_video_event
	{
	public:
		video_change_offset_event(video_group_id_t group_id, video_id_t video_id, std::chrono::nanoseconds offset) : video_group_video_event{ group_id, video_id }, offset_{ offset } {}

	private:
		std::chrono::nanoseconds offset_;

	public:
		std::chrono::nanoseconds offset() const
		{
			return offset_;
		}
	};
}
