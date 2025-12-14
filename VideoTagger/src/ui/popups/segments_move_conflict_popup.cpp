#include "pch.hpp"
#include "segments_move_conflict_popup.hpp"
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>
#include <events/timeline/segments_reject_move_event.hpp>
#include <events/timeline/segments_approve_move_event.hpp>

namespace vt::ui
{
	segments_move_conflict_popup::segments_move_conflict_popup(std::optional<bool*> open) :
		modal_popup{ "Segment Move Conflict", open, ImGuiWindowFlags_NoTitleBar }
	{
	}

	void segments_move_conflict_popup::on_display()
	{
		cancelled_ = true;
	}

	void segments_move_conflict_popup::on_render()
	{
		ui::text message(ctx_.lang->get("segments_move_conflict_popup_message"));
		close_on_escape();
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
					if (!move_event_data_.has_value())
					{
						close();
						break;
					}

					cancelled_ = false;
					ctx_.dispatch_event<segments_approve_move_event>(
						move_event_data_->storage(), move_event_data_->segments(), move_event_data_->move_part(), move_event_data_->move_offset()
					);
					close();
				}
				break;
				default: close(); break;
			}
			}, true);
	}

	void segments_move_conflict_popup::on_close()
	{
		if (cancelled_ and move_event_data_.has_value())
		{
			ctx_.dispatch_event<segments_reject_move_event>(move_event_data_->storage(), move_event_data_->segments());
		}
	}
}
