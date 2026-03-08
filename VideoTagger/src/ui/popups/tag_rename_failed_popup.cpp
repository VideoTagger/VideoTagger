#include "tag_rename_failed_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/text.hpp>

namespace vt::ui
{
	tag_rename_failed_popup::tag_rename_failed_popup(const tag_renamed_event& rename_event, std::optional<bool*> open) :
		modal_popup{ "tag-rename-failed", open }, rename_event_data_{ rename_event.storage(), rename_event.tag_name(), rename_event.new_name(), rename_event.rename_result() }
	{
	}

	void tag_rename_failed_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.tag_rename_failed.title"));
	}

	void tag_rename_failed_popup::on_render()
	{
		text message(ctx_.lang->get_template("popup.tag_rename_failed.message", rename_event_data_.tag_name(), rename_event_data_.new_name()));
		message.render();

		std::string error_text;
		switch (rename_event_data_.rename_result().validation_result)
		{
			case vt::tag_validate_result::already_exists: error_text = fmt::format("Tag \"{}\" already exists", rename_event_data_.new_name()); break;
			case vt::tag_validate_result::invalid_name: error_text = "Invalid name"; break;
			case vt::tag_validate_result::too_long: error_text = fmt::format("Name can be at most {} characters long", tag_storage::max_tag_name_length); break;
			default: error_text = "Invalid name"; break;
		}
		text reason(error_text);
		reason.render_disabled();

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
		};
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			close();
		});
	}
}
