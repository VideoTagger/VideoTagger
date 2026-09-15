#pragma once
#include <events/event.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	///@brief Event triggered when a segment drag operation is finished
	struct end_segment_drag_event : public multi_segment_event
	{
	public:
		end_segment_drag_event(segment_storage& storage, const segment_id_map& segments, segment_part grab_part, timestamp final_offset) :
			multi_segment_event{ storage, segments }, grab_part_{ grab_part }, final_offset_{ final_offset } {}

	private:
		segment_part grab_part_;
		timestamp final_offset_;

	public:
		///@return The part of the segment being grabbed
		constexpr segment_part grab_part() const
		{
			return grab_part_;
		}

		///@return The final offset of the drag operation
		constexpr timestamp final_offset() const
		{
			return final_offset_;
		}
	};
}
