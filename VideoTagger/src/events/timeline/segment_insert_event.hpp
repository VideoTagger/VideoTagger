#pragma once
#include <string>
#include <optional>
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	//TODO: maybe insert request and insert should have some id to connect them

	/**
	 * @brief Event triggered after resolving a segments_insert_request_event.
	 * 
	 * If inserted() returns false, the insertion was cancelled, otherwise a new segment has been inserted.
	 */
	struct segment_insert_event : public event
	{
		segment_insert_event(segment_storage& storage, const std::string& tag, timestamp start, timestamp end, segment_id id, bool inserted) :
			segment_storage_{ &storage }, tag_{ tag }, start_{ start }, end_{ end }, id_{ id }, inserted_{ inserted } {}

		segment_insert_event(segment_storage& storage, const std::optional<std::string>& tag, timestamp start, timestamp end, segment_id id, bool inserted) :
			segment_storage_{ &storage }, tag_{ tag }, start_{ start }, end_{ end }, id_{ id }, inserted_{ inserted } {
		}

	private:
		bool inserted_{};
		std::optional<std::string> tag_;
		segment_storage* segment_storage_;
		segment_id id_;
		timestamp start_;
		timestamp end_;

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

		///@return ID of the inserted segment, if the insertion was cancelled, the value is undefined
		constexpr segment_id id() const
		{
			return id_;
		}

		///@return Whether the segment was inserted or the insertion was cancelled
		constexpr bool inserted() const
		{
			return inserted_;
		}

		///@brief Type of the inserted segment
		constexpr tag_segment_type segment_type() const
		{
			return start_ == end_ ? tag_segment_type::timestamp : tag_segment_type::segment;
		}

		///@return Start timestamp of the inserted segment
		constexpr timestamp start() const
		{
			return start_;
		}

		///@return End timestamp of the inserted segment
		constexpr timestamp end() const
		{
			return end_;
		}
	};
}
