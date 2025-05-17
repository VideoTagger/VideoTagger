#include "pch.hpp"
#include "new_language_popup.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>
#include <utils/string.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	new_language_popup::new_language_popup(std::optional<bool*> open) : modal_popup{ "New Language", open, ImGuiWindowFlags_NoTitleBar },
		name_input{ "##LanuguageName", "English", [](const std::string& input)
		{
			auto trimmed = utils::string::trim_whitespace(input);
			return !trimmed.empty() and std::find_if(ctx_.lang_packs.begin(), ctx_.lang_packs.end(), [&](const auto& lang)
			{
				return lang->name() == trimmed;
			}) == ctx_.lang_packs.end();
		}},
		filename_input{ "##LanguageFilename", "en_US", [](const std::string& input)
		{
			auto trimmed = utils::string::trim_whitespace(input);
			return !trimmed.empty() and std::find_if(ctx_.lang_packs.begin(), ctx_.lang_packs.end(), [&](const auto& lang)
			{
				return lang->filename() == trimmed;
			}) == ctx_.lang_packs.end();
		}} {}

	void new_language_popup::on_display()
	{
		name_input.clear();
		filename_input.clear();
		name_input.focus();
	}

	void new_language_popup::on_render()
	{
		name_input.render_with_label(ctx_.lang->get("name"));
		filename_input.render_with_label(ctx_.lang->get("filename"));
		close_on_escape();

		widgets::vertical_item_spacer();

		bool valid = name_input.is_valid() and filename_input.is_valid();

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("done") },
			{ 1, ctx_.lang->get("cancel") },
		};

		ui::button_bar<int>::render(buttons, valid, [&](int id)
		{
			switch (id)
			{
				case 0:
				{
					auto lang = std::make_shared<lang_pack>(name_input.trimmed_input(), filename_input.trimmed_input());
					lang->save(ctx_.lang_dir_filepath);
					ctx_.instert_lang_pack(lang);
					close();
				}
				break;
				default: close(); break;
			}
		});
	}
}
