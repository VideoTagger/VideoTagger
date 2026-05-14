#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_start_download_request_event : public video_event
	{
		video_start_download_request_event(video_id_t video_id) : video_event(video_id) {}
	};
}
