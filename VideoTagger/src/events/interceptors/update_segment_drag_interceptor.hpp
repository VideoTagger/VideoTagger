#pragma once
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/event_interceptor.hpp>
#include <tags/tag_timeline.hpp>
#include <core/app_context.hpp>
#include <widgets/timeline.hpp>

namespace vt
{
	struct update_segment_drag_interceptor : public event_interceptor<update_segment_drag_event>
	{
		constexpr update_segment_drag_interceptor(event_interceptor_handle handle) : event_interceptor{ handle } {}

		virtual bool on_dispatch(update_segment_drag_event& event) override
		{
			auto [min_ts, max_ts] = event.min_max_timestamp();
			const auto& state_ = ctx_.get_window<widgets::timeline>().state();
			auto current_offset = event.current_offset();

			if (event.grab_part() & segment_part::left)
			{
				auto current_min_pos = min_ts + current_offset;
				if (current_min_pos < state_.min_ts)
				{
					current_offset -= current_min_pos - state_.min_ts;
				}
				else if (current_min_pos > state_.max_ts)
				{
					current_offset -= current_min_pos - state_.max_ts;
				}
			}
			if (event.grab_part() & segment_part::right)
			{
				auto current_max_pos = max_ts + current_offset;
				if (current_max_pos < state_.min_ts)
				{
					current_offset -= current_max_pos - state_.min_ts;
				}
				else if (current_max_pos > state_.max_ts)
				{
					current_offset -= current_max_pos - state_.max_ts;
				}
			}

			event.set_current_offset(current_offset);

			return true;
		}
	};
}
