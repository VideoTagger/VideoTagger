#pragma once
#include "video_group_video_event.hpp"
#include <core/types.hpp>

namespace vt
{
	struct video_open_properties_request_event : public video_group_video_event
	{
	public:
		video_open_properties_request_event(video_group_id_t group_id, video_id_t video_id, std::chrono::nanoseconds offset) : video_group_video_event{ group_id, video_id }, offset_{ offset } {}

	private:
		std::chrono::nanoseconds offset_;

	public:
		std::chrono::nanoseconds offset() const
		{
			return offset_;
		}
	};
}
