#include "pch.hpp"
#include "segment_insert_conflict_popup.hpp"
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>

namespace vt::ui
{
	segment_insert_conflict_popup::segment_insert_conflict_popup(std::optional<bool*> open) :
		modal_popup{ "Segment Insert Conflict", open, ImGuiWindowFlags_NoTitleBar }
	{
	}

	void segment_insert_conflict_popup::on_display()
	{
		cancelled_ = true;
	}

	void segment_insert_conflict_popup::on_render()
	{
		close_on_escape();

		ui::text message(ctx_.lang->get("segment_insert_conflict_popup_message"));
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
					if (!insert_request_event_data_.has_value())
					{
						close();
						break;
					}

					cancelled_ = false;
					close();
				}
				break;
				default: close(); break;
				}
			}, true);
	}

	void segment_insert_conflict_popup::on_close()
	{
	}

	bool segment_insert_conflict_popup::accepted() const
	{
		return !cancelled_;
	}
}
