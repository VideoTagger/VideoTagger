#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when the move operation for one or more segments has been rejected (e.g. the user rejected a merge operation)
	struct segments_reject_move_event : public multi_segment_event
	{
		segments_reject_move_event(segment_storage& storage, const segment_id_map& segments) :
			multi_segment_event(storage, segments) {}
	};
}
