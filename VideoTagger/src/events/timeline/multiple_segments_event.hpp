#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/// @brief Base class for events related to multiple segments
	struct multiple_segments_event : public event
	{
		multiple_segments_event(segment_storage& storage, const segment_id_map& segments) : segment_storage_{ &storage }, segments_{ segments } {}

		/**
		 * @brief Get the segments associated with this event
		 * 
		 * @return Map of tag names to sets of segment IDs.
		 */
		constexpr const segment_id_map& segments() const
		{
			return segments_;
		}

		/**
		 * @brief Get the segment storage managing the segment associated with this event
		 *
		 * @return Reference to the segment storage.
		 */
		constexpr segment_storage& storage() const
		{
			return *segment_storage_;
		}

	private:
		segment_id_map segments_;
		segment_storage* segment_storage_;
	};
}
