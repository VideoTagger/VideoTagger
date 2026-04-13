#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_deleted_event : public video_event
	{
	public:
		video_deleted_event(video_id_t video_id, bool deleted) :
			video_event(video_id), deleted_{ deleted } {}

	private:
		bool deleted_{};

	public:
		///@return Whether the video was deleted
		bool deleted() const
		{
			return deleted_;
		}
	};
}
