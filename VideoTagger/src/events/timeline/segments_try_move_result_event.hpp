#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/**
	 * @brief Event triggered after resolving a segments_try_move_event.
	 * 
	 * If approved() returns false, the move should be cancelled and the segments should remain in their original position, otherwise the move should be applied.
	 * The actual move operation is perferomed as a result of processing this event.
	 * This event can be dispatched without a prior segments_try_move_event to bypass checks, in which case approved should always return true.
	 */
	struct segments_try_move_result_event : public multi_segment_event
	{
		segments_try_move_result_event(segment_storage& storage, const segment_id_map& segments, segment_part move_part, timestamp move_offset, bool approved) :
			multi_segment_event(storage, segments), move_part_{ move_part }, move_offset_{ move_offset }, approved_{ approved } {}

	private:
		segment_part move_part_;
		timestamp move_offset_;
		bool approved_{};

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

		///@return Whether the move was approved
		constexpr bool approved() const
		{
			return approved_;
		}
	};
}
