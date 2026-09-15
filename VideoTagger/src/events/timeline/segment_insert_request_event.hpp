#pragma once
#include <optional>
#include <string>
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
		segment_insert_request_event(segment_storage& storage, const std::string& tag, timestamp start, timestamp end, bool user_customization, bool ignore_conflicts) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ start }, end_{ end }, user_customization_{ user_customization }, ignore_conflicts_{ ignore_conflicts } {}
		segment_insert_request_event(segment_storage& storage, const std::string& tag, timestamp ts, bool user_customization, bool ignore_conflicts) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ ts }, end_{ ts }, user_customization_{ user_customization }, ignore_conflicts_{ ignore_conflicts } {}

		segment_insert_request_event(segment_storage& storage, const std::optional<std::string>& tag, timestamp start, timestamp end, bool user_customization, bool ignore_conflicts) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ start }, end_{ end }, user_customization_{ tag.has_value() ? user_customization : true }, ignore_conflicts_{ ignore_conflicts } {
		}
		segment_insert_request_event(segment_storage& storage, const std::optional<std::string>& tag, timestamp ts, bool user_customization, bool ignore_conflicts) :
			tag_{ tag }, segment_storage_{ &storage }, start_{ ts }, end_{ ts }, user_customization_{ tag.has_value() ? user_customization : true }, ignore_conflicts_{ ignore_conflicts } {
		}
		
		segment_insert_request_event(segment_storage& storage, timestamp start, timestamp end, bool ignore_conflicts) :
			segment_storage_{ &storage }, start_{ start }, end_{ end }, user_customization_{ true }, ignore_conflicts_{ ignore_conflicts } {}
		segment_insert_request_event(segment_storage& storage, timestamp ts, bool ignore_conflicts) :
			segment_storage_{ &storage }, start_{ ts }, end_{ ts }, user_customization_{ true }, ignore_conflicts_{ ignore_conflicts } {}

	private:
		std::optional<std::string> tag_;
		segment_storage* segment_storage_;
		timestamp start_;
		timestamp end_;
		bool user_customization_{};
		bool ignore_conflicts_{};

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

		///@return Whether the user is supposed to customize the segment parameters (e.g., by showing a popup to the user)
		constexpr bool user_customization() const
		{
			return user_customization_;
		}

		///@return Whether conflicts with existing segments should be ignored (i.e., the new segment should be inserted without asking the user even if it overlaps with existing segments)
		constexpr bool ignore_conflicts() const
		{
			return ignore_conflicts_;
		}
	};
}
