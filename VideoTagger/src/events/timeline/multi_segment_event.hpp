#pragma once
#include <events/event.hpp>
#include <utility>
#include <tags/tag_timeline.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	///@brief Base class for events related to multiple segments
	struct multi_segment_event : public event
	{
	public:
		multi_segment_event(segment_storage& storage, const segment_id_map& segments) : segment_storage_{ &storage }, segments_{ segments } {}

	private:
		segment_id_map segments_;
		segment_storage* segment_storage_;

	public:
		 ///@return Segments associated with the event
		constexpr const segment_id_map& segments() const
		{
			return segments_;
		}

		///@return Reference to the segment storage
		constexpr segment_storage& storage() const
		{
			return *segment_storage_;
		}

		///@return The minimum and maximum timestamp among all segments associated with the event
		std::pair<timestamp, timestamp> min_max_timestamp() const
		{
			return min_max_segment_timestamps(*segment_storage_, segments_);
		}
	};
}
