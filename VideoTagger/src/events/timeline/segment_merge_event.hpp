#pragma once
#include "segment_event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	/// @brief Event triggered when a segment is merge into another segment
	struct segment_merge_event : private segment_event
	{
		segment_merge_event(segment_storage& storage, const std::string& tag, segment_id merged_id, segment_id merged_into_id) : 
			segment_event(storage, tag, merged_id), merged_into_id_{ merged_into_id } {}

		using segment_event::storage;
		using segment_event::tag;

		/**
		 * @brief Get the ID of the segment that was merged into another segment
		 * 
		 * @return ID of the merged segment.
		 */
		constexpr segment_id merged_id() const
		{
			return id();
		}

		/**
		 * @brief Get the ID of the segment that the other segment was merged into
		 * 
		 * @return ID of the segment that the other segment was merged into.
		 */
		constexpr segment_id merged_into_id() const
		{
			return merged_into_id_;
		}

	private:
		segment_id merged_into_id_;
	};
}
