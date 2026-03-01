#include "pch.hpp"
#include "segments_move_conflict_popup.hpp"
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segments_moved_event.hpp>
#include <events/player/playback_suspend_request_event.hpp>
#include <events/player/playback_resume_request_event.hpp>

namespace vt::ui
{
	segments_move_conflict_popup::segments_move_conflict_popup(const segments_move_request_event& event_data, segment_id_map conflicting_segments, std::optional<bool*> open) :
		modal_popup{ /*ctx_.lang->get("popup.segments_move_conflict.title")*/ "Move Segment Conflict", open, ImGuiWindowFlags_NoTitleBar},
		move_request_event_data_{ event_data.storage(), event_data.segments(), event_data.move_part(), event_data.move_offset(), event_data.ignore_conflicts() },
		conflicting_segments_{ conflicting_segments }
	{
	}

	void segments_move_conflict_popup::on_display()
	{
		ctx_.dispatch_event<playback_suspend_request_event>(move_request_event_data_.source(), ctx_.get_window<widgets::video_player>());
	}

	void segments_move_conflict_popup::on_render()
	{
		close_on_escape();

		ui::text message(ctx_.lang->get("popup.segments_move_conflict.message"));
		message.render();
		ui::vertical_item_spacer();
		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				accepted_ = true;
				close();
				ctx_.dispatch_event<segments_move_request_event>(
					move_request_event_data_.source(), move_request_event_data_.storage(), move_request_event_data_.segments(),
					move_request_event_data_.move_part(), move_request_event_data_.move_offset(), true
				);
			}
			break;
			default: close(); break;
			}
		}, true);
	}

	void segments_move_conflict_popup::on_close()
	{
		if (!accepted_)
		{
			ctx_.dispatch_event<segments_moved_event>(
				move_request_event_data_.source(), move_request_event_data_.storage(), move_request_event_data_.segments(),
				move_request_event_data_.move_part(), move_request_event_data_.move_offset(), false
			);
		}

		ctx_.dispatch_event<playback_resume_request_event>(move_request_event_data_.source(), ctx_.get_window<widgets::video_player>());
	}
}
