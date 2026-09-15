#pragma once
#include "segment_event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when a segment is merge into another segment
	struct segment_merged_event : public segment_event
	{
	public:
		segment_merged_event(segment_storage& storage, const std::string& tag, segment_id merged_id, segment_id merged_into_id) : 
			segment_event(storage, tag, merged_id), merged_into_id_{ merged_into_id } {}

	private:
		segment_id merged_into_id_;

	public:
		///@return ID of the segment that was merged into another segment
		constexpr segment_id merged_id() const
		{
			return id();
		}

		///return ID of the segment that the segment was merged into.
		constexpr segment_id merged_into_id() const
		{
			return merged_into_id_;
		}
	};
}
