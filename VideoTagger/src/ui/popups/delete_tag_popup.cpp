#include "delete_tag_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/text.hpp>
#include <ui/widgets/button_bar.hpp>

#include <events/tags/tag_delete_request_event.hpp>

namespace vt::ui
{
	delete_tag_popup::delete_tag_popup(event_source source, const std::string& tag_name, std::optional<bool*> open) :
		modal_popup("delete-tag-popup", open), event_source_{ source }, tag_name_{ tag_name }
	{
	}

	void delete_tag_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.delete_tag.title"));
	}

	void delete_tag_popup::on_render()
	{
		ui::text message(ctx_.lang->get_template("popup.delete_tag.message", tag_name_));
		message.render();

		//TODO: should be colored
		ui::text warning(ctx_.lang->get("popup.delete_tag.warning"));
		warning.render();

		//ImGui::TextColored({ 1.f, 170.f / 255.f, 50.f / 255.f, 1.f }, "All segments associated with this tag will be deleted as well!");

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
				auto& tags = ctx_.current_project->tags;
				ctx_.dispatch_event<tag_delete_request_event>(event_source_, tags, tag_name_);
				close();
				break;
			}
			default: close(); break;
			}
		});
	}
}
