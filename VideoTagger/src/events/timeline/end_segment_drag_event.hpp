#pragma once
#include <events/event.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	/// @brief Event triggered when a segment drag operation is finished
	struct end_segment_drag_event : public event
	{
		end_segment_drag_event(timestamp final_offset) : final_offset_{ final_offset } {}

		/**
		 * @brief Get the final offset (timestamp) of the drag operation
		 *
		 * @return The final offset of the drag operation.
		 */
		constexpr timestamp final_offset() const
		{
			return final_offset_;
		}

	private:
		timestamp final_offset_;
	};
}
