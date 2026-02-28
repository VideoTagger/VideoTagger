#include "pch.hpp"
#include "segment_insert_conflict_popup.hpp"
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>

#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_insert_event.hpp>
#include <events/player/playback_changed_event.hpp>

namespace vt::ui
{
	segment_insert_conflict_popup::segment_insert_conflict_popup(const segment_insert_request_event& event_data,
		const std::set<segment_id>& conflicting_segments, std::optional<bool*> open) :
		modal_popup{ /*ctx_.lang->get("popup.segment_insert_conflict.title")*/ "Insert Segment Conflict", open, ImGuiWindowFlags_NoTitleBar},
		insert_request_event_data_{ event_data.storage(), event_data.tag(), event_data.start(), event_data.end(), event_data.user_customization(), event_data.ignore_conflicts() },
		conflicting_segments_{ conflicting_segments }
	{
	}

	void segment_insert_conflict_popup::on_display()
	{
		//TODO: this and what's in on_close is repeated in multiple popups, should be refactored

		auto& player = ctx_.get_window<widgets::video_player>();
		if (player.is_playing())
		{
			player.set_playing(false); //TODO: remove when player events work

			ctx_.dispatch_event<playback_changed_event>(insert_request_event_data_.source(), player, false);
			paused_player_ = true;
		}
	}

	void segment_insert_conflict_popup::on_render()
	{
		close_on_escape();

		ui::text message(ctx_.lang->get("popup.segment_insert_conflict.message"));
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
				ctx_.dispatch_event<segment_insert_request_event>(
					insert_request_event_data_.source(), insert_request_event_data_.storage(), insert_request_event_data_.tag(),
					insert_request_event_data_.start(), insert_request_event_data_.end(), false, true
				);
			}
			break;
			default: close(); break;
			}
		}, true);
	}

	void segment_insert_conflict_popup::on_close()
	{
		if (!accepted_)
		{
			ctx_.dispatch_event<segment_insert_event>(
				insert_request_event_data_.source(), insert_request_event_data_.storage(), insert_request_event_data_.tag(),
				insert_request_event_data_.start(), insert_request_event_data_.end(), invalid_segment_id, false
			);
		}

		auto& player = ctx_.get_window<widgets::video_player>();
		if (paused_player_)
		{
			player.set_playing(true); //TODO: remove when player events work

			ctx_.dispatch_event<playback_changed_event>(insert_request_event_data_.source(), player, true);
			paused_player_ = false;
		}
	}
}
