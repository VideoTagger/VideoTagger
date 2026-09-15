#pragma once
#include "events/event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	struct segment_select_all_request_event : public event
	{
		segment_select_all_request_event(segment_storage& storage) : storage_{ &storage } {}

	private:
		segment_storage* storage_;

	public:
		segment_storage& storage() const
		{
			return *storage_;
		}
	};

}
