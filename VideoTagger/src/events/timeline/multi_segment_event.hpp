#pragma once
#include <events/event.hpp>
#include <utility>
#include <tags/tag_timeline.hpp>

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
			timestamp min_timestamp = timestamp::max();
			timestamp max_timestamp = timestamp::min();
			for (auto& [tag, segment_ids] : segments_)
			{
				auto& tag_segments = segment_storage_->at(tag);
				for (auto& segment_id : segment_ids)
				{
					auto& segment = tag_segments.at(segment_id);
					min_timestamp = std::min(min_timestamp, segment.start);
					max_timestamp = std::max(max_timestamp, segment.end);
				}
			}

			return { min_timestamp, max_timestamp };
		}
	};
}
