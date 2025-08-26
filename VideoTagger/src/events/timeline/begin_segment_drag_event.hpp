#pragma once
#include <events/timeline/multiple_segments_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/// @brief Event triggered when a segment drag operation begins
	struct begin_segment_drag_event : public multiple_segments_event
	{
		begin_segment_drag_event(segment_storage& storage, const segment_id_map& segments, segment_part grab_part, timestamp start_position) :
			multiple_segments_event(storage, segments), grab_part_{ grab_part }, start_position_{ start_position } {}

		/**
		 * @brief Get which part of the segment is being grabbed
		 * 
		 * @return The part of the segment being grabbed.
		 */
		constexpr segment_part grab_part() const
		{
			return grab_part_;
		}

		/**
		 * @brief Get the start position (timestamp) of the drag operation
		 * 
		 * @return The start position of the drag operation.
		 */
		constexpr timestamp start_position() const
		{
			return start_position_;
		}

	private:
		segment_part grab_part_{ segment_part::none };
		timestamp start_position_;
	};
}
