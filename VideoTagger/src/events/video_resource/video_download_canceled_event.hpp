#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_download_canceled_event : public video_event
	{
		video_download_canceled_event(video_id_t video_id) : video_event(video_id) {}
	};
}
