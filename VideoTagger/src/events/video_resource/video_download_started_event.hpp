#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_download_started_event : public video_event
	{
		video_download_started_event(video_id_t video_id) : video_event(video_id) {}
	};
}
