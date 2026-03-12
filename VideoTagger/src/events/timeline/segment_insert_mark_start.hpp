#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	struct segment_insert_mark_start : public event
	{
	public:
		segment_insert_mark_start(uint64_t mark_id, segment_storage& storage, const std::string& tag, timestamp ts) :
			mark_id_{ mark_id }, tag_{ tag }, segment_storage_{ &storage }, timestamp_{ ts } {}

		segment_insert_mark_start(uint64_t mark_id, segment_storage& storage, const std::optional<std::string>& tag, timestamp ts) :
			mark_id_{ mark_id }, tag_{ tag }, segment_storage_{ &storage }, timestamp_{ ts } {}

		segment_insert_mark_start(uint64_t mark_id, segment_storage& storage, timestamp ts) :
			mark_id_{ mark_id }, segment_storage_{ &storage }, timestamp_{ ts } {}

	private:
		std::optional<std::string> tag_;
		segment_storage* segment_storage_;
		timestamp timestamp_;
		uint64_t mark_id_{};

	public:
		///@return Tag name
		constexpr const std::optional<std::string>& tag() const
		{
			return tag_;
		}

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
	};
}
