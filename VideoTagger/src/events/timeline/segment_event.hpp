#pragma once
#include <events/event.hpp>
#include <tags/tag_timeline.hpp>

namespace vt
{
	/// @brief Base class for events related to a single segment
	struct segment_event : public event
	{
		segment_event(segment_storage& storage, const std::string& tag, segment_id id) : segment_storage_{ &storage }, tag_ { tag }, id_{ id } {}

		/**
		 * @brief Get the ID of the segment associated with this event
		 * 
		 * @return The segment ID.
		 */
		constexpr segment_id id() const
		{
			return id_;
		}

		/**
		 * @brief Get the tag name to which the segment associated with this event belongs
		 * 
		 * @return The tag name.
		 */
		constexpr const std::string& tag() const
		{
			return tag_;
		}

		/**
		 * @brief Get the segment storage managing the segment associated with this event
		 * 
		 * @return Reference to the segment storage.
		 */
		constexpr segment_storage& storage() const
		{
			return *segment_storage_;
		}

	private:
		std::string tag_;
		segment_id id_;
		segment_storage* segment_storage_;
	};
}
