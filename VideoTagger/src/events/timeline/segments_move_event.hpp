#pragma once
#include <events/timeline/multiple_segments_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/// @brief Event triggered when one or more segments have been moved
	struct segments_move_event : public multiple_segments_event
	{
		segments_move_event(segment_storage& storage, const segment_id_map& segments, segment_part move_part, timestamp move_offset) :
			multiple_segments_event(storage, segments), move_part_{ move_part }, move_offset_{ move_offset } {}

		/**
		 * @brief Get which part of the segment was moved
		 * 
		 * @return The part of the segment that was moved.
		 */
		constexpr segment_part move_part() const
		{
			return move_part_;
		}

		/**
		 * @brief Get the offset (timestamp) by which the segments were moved
		 * 
		 * @return The offset by which the segments were moved.
		 */
		constexpr timestamp move_offset() const
		{
			return move_offset_;
		}

	private:
		segment_part move_part_{ segment_part::none };
		timestamp move_offset_;
	};
}
