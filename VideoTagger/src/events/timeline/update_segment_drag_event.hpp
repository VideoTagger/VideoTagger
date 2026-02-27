#pragma once
#include <events/event.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	///@brief Event triggered when a segment drag operation is updated (moved)
	struct update_segment_drag_event : public multi_segment_event
	{
	public:
		update_segment_drag_event(segment_storage& storage, const segment_id_map& segments, segment_part grab_part, timestamp current_offset) :
			multi_segment_event{ storage, segments }, grab_part_{ grab_part }, current_offset_{ current_offset } {}

	private:
		segment_part grab_part_;
		timestamp current_offset_;

	public:
		///@return The part of the segment being grabbed
		constexpr segment_part grab_part() const
		{
			return grab_part_;
		}

		///@return The current offset of the drag operation
		constexpr timestamp current_offset() const
		{
			return current_offset_;
		}
	};
}
