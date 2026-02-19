#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/**
	 * @brief Event triggered when trying to insert a segments.
	 *
	 * The segment is not actually inserted yet and the event can be used to modify or cancel the insertion.
	 * After processing this event, a segments_try_insert_result_event should be dispatched with
	 * approved set to true if the insertion should be applied or false if the insertion should be cancelled.
	 */
	struct segment_insert_request_event : public event
	{
		segment_insert_request_event(segment_storage& storage, const std::string& tag, timestamp start, timestamp end) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ start }, end_{ end } {
		}
		segment_insert_request_event(segment_storage& storage, const std::string& tag, timestamp ts) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ ts }, end_{ ts } {
		}

	private:
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
	};
}
