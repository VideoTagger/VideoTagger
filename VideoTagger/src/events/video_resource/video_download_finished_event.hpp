#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_download_finished_event : public video_event
	{
	public:
		video_download_finished_event(video_id_t video_id, bool successful) :
			video_event(video_id), successful_{ successful } {}

	private:
		bool successful_;

	public:
		bool successful() const
		{
			return successful_;
		}
	};
}
