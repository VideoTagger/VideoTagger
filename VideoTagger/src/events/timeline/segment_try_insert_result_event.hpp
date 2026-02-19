#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/**
	 * @brief Event triggered after resolving a segments_try_insert_event.
	 * 
	 * The segment should only be inserted if approved() returns true.
	 * The actual insertion is perferomed as a result of processing this event.
	 * This event can be dispatched without a prior segments_try_insert_event to bypass checks, in which case approved() should always return true.
	 */
	struct segment_try_insert_result_event : public event
	{
		segment_try_insert_result_event(segment_storage& storage, const std::string& tag, timestamp start, timestamp end, bool approved) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ start }, end_{ end }, approved_{ approved } {}
		segment_try_insert_result_event(segment_storage& storage, const std::string& tag, timestamp ts, bool approved) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ ts }, end_{ ts }, approved_{ approved } {}

	private:
		bool approved_{};
		std::string tag_;
		segment_storage* segment_storage_;
		timestamp start_;
		timestamp end_;

	public:
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

		///@brief Type of the segment being inserted
		constexpr tag_segment_type segment_type() const
		{
			return start_ == end_ ? tag_segment_type::timestamp : tag_segment_type::segment;
		}

		///@return Start timestamp of the segment being inserted
		constexpr timestamp start() const
		{
			return start_;
		}

		///@return End timestamp of the segment being inserted
		constexpr timestamp end() const
		{
			return end_;
		}

		///@return Whether the insertion was approved
		constexpr bool approved() const
		{
			return approved_;
		}
	};
}
