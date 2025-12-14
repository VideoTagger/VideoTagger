#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when one or more segments have been moved
	struct segments_approve_move_event : public multi_segment_event
	{
		segments_approve_move_event(segment_storage& storage, const segment_id_map& segments, segment_part move_part, timestamp move_offset) :
			multi_segment_event(storage, segments), move_part_{ move_part }, move_offset_{ move_offset } {}

	private:
		segment_part move_part_;
		timestamp move_offset_;

	public:
		///@return The part of the segment that was moved
		constexpr segment_part move_part() const
		{
			return move_part_;
		}

		///@return The offset by which the segments were moved
		constexpr timestamp move_offset() const
		{
			return move_offset_;
		}
	};
}
