#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/**
	 * @brief Event triggered after resolving a segments_move_request_event.
	 * 
	 * If moved() returns false, the move was cancelled and the segments remained in their original position, otherwise the move was performed.
	 */
	struct segments_move_event : public multi_segment_event
	{
		segments_move_event(segment_storage& storage, const segment_id_map& segments, segment_part move_part, timestamp move_offset, bool moved) :
			multi_segment_event(storage, segments), move_part_{ move_part }, move_offset_{ move_offset }, moved_{ moved } {}

	private:
		segment_part move_part_;
		timestamp move_offset_;
		bool moved_{};

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

		///@return Whether the segments were moved or the move was cancelled
		constexpr bool moved() const
		{
			return moved_;
		}
	};
}
