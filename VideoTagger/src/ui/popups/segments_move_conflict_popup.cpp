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
		modal_popup{ "segments-move-conflict", open, ImGuiWindowFlags_NoTitleBar},
		move_request_event_data_{ event_data.storage(), event_data.segments(), event_data.move_part(), event_data.move_offset(), event_data.ignore_conflicts() },
		conflicting_segments_{ conflicting_segments }, event_source_{ event_data.source() }
	{
	}

	void segments_move_conflict_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.segments_move_conflict.title"));
		ctx_.dispatch_event<playback_suspend_request_event>(event_source_, ctx_.get_window<widgets::video_player>());
	}

	void segments_move_conflict_popup::on_render()
	{
		close_on_escape();

		ui::text message(ctx_.lang->get("popup.segments_move_conflict.message"));
		message.render();
		ui::vertical_item_spacer();
		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("generic.confirm") },
			{ 1, ctx_.lang->get("generic.cancel") },
		};
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				close();
				ctx_.dispatch_event<segments_move_request_event>
				(
					event_source_, move_request_event_data_.storage(), move_request_event_data_.segments(),
					move_request_event_data_.move_part(), move_request_event_data_.move_offset(), true
				);
			}
			break;
			default:
			{
				close();
				ctx_.dispatch_event<segments_moved_event>
				(
					event_source_, move_request_event_data_.storage(), move_request_event_data_.segments(),
					move_request_event_data_.move_part(), move_request_event_data_.move_offset(), false
				);
				break;
			}
			}
		}, true);
	}

	void segments_move_conflict_popup::on_close()
	{
		ctx_.dispatch_event<playback_resume_request_event>(event_source_, ctx_.get_window<widgets::video_player>());
	}
}
