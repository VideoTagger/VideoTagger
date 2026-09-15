#pragma once
#include "segment_event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when a segment is selected
	struct segment_delete_request_event : public segment_event
	{
		segment_delete_request_event(segment_storage& storage, const std::string& tag, segment_id id) : segment_event{ storage, tag, id } {}
	};
}
