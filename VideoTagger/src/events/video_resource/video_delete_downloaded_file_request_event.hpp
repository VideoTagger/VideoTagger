#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_delete_downloaded_file_request_event : public video_event
	{
	public:
		video_delete_downloaded_file_request_event(video_id_t video_id) :
			video_event(video_id) {}
	};
}
