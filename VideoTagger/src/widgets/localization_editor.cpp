#include "pch.hpp"
#include "localization_editor.hpp"
#include "icons.hpp"

namespace vt::widgets
{
	void localization_editor::render(bool& is_open, std::vector<std::shared_ptr<lang_pack>>& langs)
	{
		auto win_name = window_name();
		if (ImGui::Begin(win_name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			size_t lang_count = langs.size();

			static constexpr int table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

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
						ImGui::InputTextWithHint(fmt::format("##Localization({}.{})", lang->name(), key).c_str(), "Empty", &lang->at(key));
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
