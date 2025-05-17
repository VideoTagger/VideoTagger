#include "pch.hpp"
#include "localization_editor.hpp"
#include "icons.hpp"
#include "controls.hpp"

#include <core/app_context.hpp>
#include <utils/filesystem.hpp>


namespace vt::widgets
{
	localization_editor::localization_editor() : new_lang_popup_{ ui::new_popup<ui::new_language_popup>() }, remove_lang_popup_{ ui::new_popup<ui::remove_language_popup>() } {}

	void localization_editor::render(bool& is_open, std::vector<std::shared_ptr<lang_pack>>& langs)
	{
		const auto& style = ImGui::GetStyle();

		auto win_name = window_name();
		if (ImGui::Begin(win_name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			size_t lang_count = langs.size();

			static constexpr int table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, style.ItemSpacing.y });
			bool add_lang = icon_button(icons::add);

			tooltip("Add Language");
			ImGui::SameLine();
			if (icon_button(icons::save))
			{
				for (auto& lang : langs)
				{
					if (!lang->is_dirty()) continue;
					lang->save(ctx_.lang_dir_filepath);
					lang->set_dirty(false);
				}
			}
			tooltip("Save");
			ImGui::SameLine();
			if (icon_button(icons::download))
			{
				utils::dialog_filters filters{ utils::dialog_filter{ "VideoTagger Lang Pack", lang_pack::extension } };

				auto input_files = utils::filesystem::get_files({}, filters);
				if (input_files)
				{
					for (const auto& file : input_files.paths)
					{
						auto lang = lang_pack::load_from_file(file);
						if (lang.has_value())
						{
							ctx_.instert_lang_pack(std::make_shared<lang_pack>(lang.value()));
						}
					}
				}
			}
			tooltip("Import");
			ImGui::SameLine();
			if (icon_button(icons::upload))
			{
				auto output_dir = utils::filesystem::get_folder();
				if (output_dir)
				{
					debug::log("Exporting languages to: {}", output_dir.path.u8string());
					for (const auto& lang : langs)
					{
						lang->save(output_dir.path);
					}
				}
			}
			tooltip("Export");
			ImGui::SameLine();
			bool remove_lang = icon_button(icons::delete_);
			tooltip("Delete");
			ImGui::PopStyleVar();

			if (lang_count > 0 and ImGui::BeginTable("##Languages", static_cast<int>(lang_count + 1), table_flags))
			{
				auto keys_ = keys(langs);
				
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
				for (const auto& lang : langs)
				{
					float max_width{};
					for (const auto& key : keys_)
					{
						auto width = ImGui::CalcTextSize(lang->get(key).c_str()).x;
						if (width > max_width)
						{
							max_width = width;
						}
					}
					ImGui::TableSetupColumn(lang->name().c_str(), 0, max_width);
				}

				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableHeadersRow();

				for (const auto& key : keys_)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TextDisabled("%s", key.c_str());

					for (auto& lang : langs)
					{
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						if (ImGui::InputTextWithHint(fmt::format("##Localization({}.{})", lang->name(), key).c_str(), "Empty", &lang->at(key)))
						{
							lang->set_dirty(true);
						}
					}
				}
				ImGui::EndTable();
			}

			new_lang_popup_->open_and_render(add_lang);
			remove_lang_popup_->open_and_render(remove_lang);
		}
		ImGui::End();
	}

	std::string localization_editor::window_name()
	{
		return fmt::format("{} Localization Editor##Localization Editor", icons::translate);
	}

	std::vector<std::string> localization_editor::keys(const std::vector<std::shared_ptr<lang_pack>>& langs) const
	{
		std::unordered_set<std::string> key_set;
		for (const auto& lang : langs)
		{
			auto lang_keys = lang->keys();
			key_set.insert(lang_keys.begin(), lang_keys.end());
		}
		std::vector<std::string> keys(key_set.begin(), key_set.end());
		std::sort(keys.begin(), keys.end());
		return keys;
	}
}
