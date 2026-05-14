#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Base class for events related to a single segment
	struct segment_event : public event
	{
	public:
		segment_event(segment_storage& storage, const std::string& tag, segment_id id) : segment_storage_{ &storage }, tag_{ tag }, id_{ id } {}

	private:
		std::string tag_;
		segment_id id_;
		segment_storage* segment_storage_;

	public:
		///@return Segment ID
		constexpr segment_id id() const
		{
			return id_;
		}

		///@return Tag name
		constexpr const std::string& tag() const
		{
			return tag_;
		}

		///@return Reference to the segment storage
		constexpr segment_storage& storage() const
		{
			return *segment_storage_;
		}
	};
}
