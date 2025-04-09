#include "pch.hpp"
#include "localization_editor.hpp"
#include "icons.hpp"
#include "controls.hpp"
#include <core/app_context.hpp>


namespace vt::widgets
{
	void localization_editor::render(bool& is_open, std::vector<std::shared_ptr<lang_pack>>& langs)
	{
		const auto& style = ImGui::GetStyle();

		auto win_name = window_name();
		if (ImGui::Begin(win_name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			size_t lang_count = langs.size();

			static constexpr int table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, style.ItemSpacing.y });
			ImGui::BeginDisabled();
			if (icon_button(icons::add))
			{

			}
			ImGui::EndDisabled();
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
			ImGui::PopStyleVar();

			if (lang_count > 0 and ImGui::BeginTable("##Languages", static_cast<int>(lang_count + 1), table_flags))
			{
				std::vector<std::string> keys = langs.front()->keys();
				std::sort(keys.begin(), keys.end());
				
				ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
				for (const auto& lang : langs)
				{
					ImGui::TableSetupColumn(lang->name().c_str());
				}

				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableHeadersRow();

				for (const auto& key : keys)
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
		}
		ImGui::End();
	}

	std::string localization_editor::window_name()
	{
		return fmt::format("{} Localization Editor##Localization Editor", icons::translate);
	}
}
