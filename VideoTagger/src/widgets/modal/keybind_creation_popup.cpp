#define IMGUI_DEFINE_MATH_OPERATORS
#include "keybind_popup.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <core/actions.hpp>

namespace vt::widgets::modal
{
	bool keybind_creation_popup(const char* id, std::string& keybind_name, keybind& keybind, std::vector<std::shared_ptr<keybind_action>>& actions, int& selected_action)
	{
		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool win_result = ImGui::BeginPopupModal(id, nullptr, flags);
		ImGui::PopStyleVar(2);

		if (win_result)
		{
			if (ImGui::IsWindowAppearing())
			{
				keybind_name = "Keybind";
				selected_action = 0;
				keybind.key_code = -1;
				actions.clear();
				actions.push_back(std::make_shared<no_action>());
				actions.push_back(std::make_shared<add_timestamp_action>());
				keybind.action = actions[selected_action];
			}
			std::vector<const char*> action_names;
			for (const auto& action : actions)
			{
				action_names.push_back(_strdup(action->name().c_str()));
			}

			std::string name_id = "##Input" + std::string(id);
			std::string key_input_id = name_id + "Key";
			std::string key_combination = keybind.name();
			ImGui::Text("Keybind Name");
			ImGui::InputText(name_id.c_str(), &keybind_name);
			ImGui::Text("Press the desired key combination");
			ImGui::BeginDisabled();
			ImGui::InputText(key_input_id.c_str(), &key_combination, ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();
			ImGui::Text("Action");
			std::string combo_id = "##Combo" + std::string(id);
			if (ImGui::Combo(combo_id.c_str(), &selected_action, action_names.data(), static_cast<int>(actions.size())))
			{
				keybind.action = actions[selected_action];
			}
			ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
			if (ImGui::BeginTable("##KeybindPropertiesBackground", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
			{
				keybind.action->render_properties(false);
				ImGui::EndTable();
			}
			ImGui::PopStyleColor();
			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("Add"))
			{
				result = (keybind.key_code != -1);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") or ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}
}
