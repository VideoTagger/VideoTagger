#pragma once
#include <events/timeline/multi_segment_event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when a segment drag operation begins
	struct begin_segment_drag_event : public multi_segment_event
	{
	public:
		begin_segment_drag_event(segment_storage& storage, const segment_id_map& segments, segment_part grab_part, timestamp start_position) :
			multi_segment_event{ storage, segments }, grab_part_{ grab_part }, start_position_{ start_position } {
		}

	private:
		segment_part grab_part_;
		timestamp start_position_;

	public:
		///@return The part of the segment being grabbed
		constexpr segment_part grab_part() const
		{
			return grab_part_;
		}

		///@return The start position of the drag operation
		constexpr timestamp start_position() const
		{
			return start_position_;
		}
	};
}
