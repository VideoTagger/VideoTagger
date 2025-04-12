#include "pch.hpp"
#include "new_language_popup.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>
#include <utils/string.hpp>

namespace vt::ui
{
	new_language_popup::new_language_popup(std::optional<bool*> open) : modal_popup{ "New Language", open },
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

	void new_language_popup::on_render()
	{
		if (ImGui::IsWindowAppearing())
		{
			name_input.clear();
			filename_input.clear();
			name_input.focus();
		}

		name_input.render_with_label("Name");
		filename_input.render_with_label("Filename");
		close_on_escape();

		widgets::vertical_item_spacer();

		if (ImGui::Button(ctx_.lang->get("cancel").c_str()))
		{
			close();
		}

		ImGui::SameLine();

		bool valid = name_input.is_valid() and filename_input.is_valid();
		ImGui::BeginDisabled(!valid);
		if (ImGui::Button(ctx_.lang->get("done").c_str()) or (valid and ImGui::IsWindowFocused() and ImGui::IsKeyPressed(ImGuiKey_Enter)))
		{
			auto lang = std::make_shared<lang_pack>(name_input.trimmed_input(), filename_input.trimmed_input());
			lang->save(ctx_.lang_dir_filepath);
			ctx_.instert_lang_pack(lang);
			close();
		}
		ImGui::EndDisabled();
	}
}
