#pragma once

#include "video_event.hpp"

namespace vt
{
	struct video_load_thumbnail_request_event : public video_event
	{
	public:
		video_load_thumbnail_request_event(video_id_t video_id, bool ignore_cache, bool cache_result) :
			video_event{ video_id }, ignore_cache_{ ignore_cache }, cache_result_{ cache_result } {}

	private:
		bool ignore_cache_;
		bool cache_result_;

	public:
		bool ignore_cache() const
		{
			return ignore_cache_;
		}

		bool cache_result() const
		{
			return cache_result_;
		}
	};
}
