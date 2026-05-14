#pragma once
#include "events/event.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
	struct segment_deselect_all_request_event : public event
	{
		segment_deselect_all_request_event(segment_storage& storage, const segment_id_map& excluded = {}) :
			storage_{ &storage }, excluded_{ excluded } {}

	private:
		segment_storage* storage_;
		segment_id_map excluded_;

	public:
		segment_storage& storage() const
		{
			return *storage_;
		}

		const segment_id_map& excluded() const
		{
			return excluded_;
		}

		bool is_excluded(const std::string& tag_name, segment_id segment) const
		{
			return segment_id_map_contains(excluded_, tag_name, segment);
		}
	};

}
