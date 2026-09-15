#pragma once
#include "segment_event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	///@brief Event triggered when a segment is selected
	struct segment_deleted_event : public segment_event
	{
		segment_deleted_event(segment_storage& storage, const std::string& tag, segment_id id, bool deleted) :
			segment_event{ storage, tag, id }, deleted_{ deleted } {}

	private:
		bool deleted_{};

	public:
		///@return Whether the segment was deleted
		constexpr bool deleted() const
		{
			return deleted_;
		}
	};
}
