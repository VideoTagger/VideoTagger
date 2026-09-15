#include "rename_tag_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/text.hpp>
#include <ui/widgets/button_bar.hpp>

#include <events/tags/tag_rename_request_event.hpp>

namespace vt::ui
{
	rename_tag_popup::rename_tag_popup(event_source source, const std::string& old_name, const std::string& new_name, std::optional<bool*> open) :
		modal_popup("rename-tag-popup", open), event_source_{ source }, old_name_{ old_name }, new_name_{ new_name }
	{
	}

	void rename_tag_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.rename_tag.title"));
	}

	void rename_tag_popup::on_render()
	{
		ui::text message(ctx_.lang->get_template("popup.rename_tag_popup.message", old_name_, new_name_));
		message.render();

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
					auto& tags = ctx_.current_project->tags;
					ctx_.dispatch_event<tag_rename_request_event>(event_source_, tags, old_name_, new_name_);
					close();
					break;
				}
				default: close(); break;
			}
		});
	}
}
