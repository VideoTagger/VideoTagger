#include "pch.hpp"
#include "remove_language_popup.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	remove_language_popup::remove_language_popup(std::optional<bool*> open) : modal_popup{ "Remove Language", open, ImGuiWindowFlags_NoTitleBar }, languages_{ "##Languages", {} } {}

	void remove_language_popup::on_display()
	{
		languages_.set_items(ctx_.lang_names());
		languages_.reset();
	}

	void remove_language_popup::on_render()
	{
		languages_.render_with_label(ctx_.lang->get("language"));
		close_on_escape();
		widgets::vertical_item_spacer();

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("remove") },
			{ 1, ctx_.lang->get("cancel") },
		};

		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
				case 0:
				{
					ctx_.remove_lang_pack(languages_.selected_item());
					close();
				}
				break;
				default: close(); break;
			}
		}, true);
	}
}
