#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/**
	 * @brief Event triggered when trying to move one or more segments.
	 * 
	 * The segments are not actually moved yet and the event can be used to modify or cancel the move.
	 * After processing this event, a segments_try_move_result_event should be dispatched with
	 * approved set to true if the move should be applied or false if the move should be cancelled.
	 */
	struct segments_move_request_event : public multi_segment_event
	{
		segments_move_request_event(segment_storage& storage, const segment_id_map& segments, segment_part move_part, timestamp move_offset) :
			multi_segment_event(storage, segments), move_part_{ move_part }, move_offset_{ move_offset } {}

		segments_move_request_event(segment_storage& storage, const std::string tag, segment_id segment, segment_part move_part, timestamp move_offset) :
			multi_segment_event(storage, segment_id_map{ {tag, { segment }} }), move_part_{ move_part }, move_offset_{ move_offset } {
		}

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
