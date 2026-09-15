#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	struct segment_insert_mark_end : public event
	{
	public:
		segment_insert_mark_end(uint64_t mark_id, segment_storage& storage, timestamp ts, bool user_customization) :
			mark_id_{ mark_id }, segment_storage_{ &storage }, timestamp_{ ts }, user_customization_{ user_customization } {}

	private:
		segment_storage* segment_storage_;
		timestamp timestamp_;
		uint64_t mark_id_{};
		bool user_customization_{};

	public:
		///@return Reference to the segment storage
		constexpr segment_storage& storage() const
		{
			return *segment_storage_;
		}

		///@return Timestamp where the segment start was marked
		constexpr timestamp timestamp() const
		{
			return timestamp_;
		}

		///@return ID of the mark, used to match the start and end marks
		constexpr uint64_t mark_id() const
		{
			return mark_id_;
		}

		///@return Whether the user is supposed to customize the segment parameters (e.g., by showing a popup to the user)
		constexpr bool user_customization() const
		{
			return user_customization_;
		}
	};
}
