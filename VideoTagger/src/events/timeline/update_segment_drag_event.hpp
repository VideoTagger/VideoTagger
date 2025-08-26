#pragma once
#include <events/event.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	/// @brief Event triggered when a segment drag operation is updated (moved)
	struct update_segment_drag_event : public event
	{
		update_segment_drag_event(timestamp current_offset) : current_offset_{ current_offset } {}

		/**
		 * @brief Get the current offset (timestamp) of the drag operation
		 * 
		 * @return The current offset of the drag operation.
		 */
		constexpr timestamp current_offset() const
		{
			return current_offset_;
		}

	private:
		timestamp current_offset_;
	};
}
