#pragma once

#include <events/event.hpp>
#include <video/video_resource.hpp>

namespace vt
{
	struct video_event : public event
	{
	public:
		video_event(video_id_t video_id) : video_id_(video_id) {}

	private:
		video_id_t video_id_;

	public:
		video_id_t video_id() const
		{
			return video_id_;
		}
	};
}
