#include "pch.hpp"
#include "keybind_options_popup.hpp"
#include "keybind_popup.hpp"
#include <core/debug.hpp>
#include <core/actions.hpp>

namespace vt::widgets::modal
{
	bool keybind_options_popup(const char* id, std::string& keybind_name, keybind& keybind, std::vector<std::shared_ptr<keybind_action>>& actions, int& selected_action, keybind_options_config config, const std::function<bool(const std::string&, const vt::keybind&, keybind_validator_mode)>& validator, keybind_validator_mode validator_mode)
	{
		if (validator == nullptr)
		{
			debug::panic("Keybind validator was nullptr");
			return false;
		}
		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool win_result = ImGui::BeginPopupModal(id, nullptr, flags);
		ImGui::PopStyleVar(2);

		bool creation_mode = (uint8_t)(config) & (uint8_t)(keybind_options_config::creation_mode);
		bool show_keybind_field = (uint8_t)(config) & (uint8_t)(keybind_options_config::show_keybind_field);
		bool show_name_field = (uint8_t)(config) & (uint8_t)(keybind_options_config::show_name_field);
		bool show_action_field = (uint8_t)(config) & (uint8_t)(keybind_options_config::show_action_field);
		bool show_save_button = (uint8_t)(config) & (uint8_t)(keybind_options_config::show_save_button);

		if (win_result)
		{
			if (ImGui::IsWindowAppearing())
			{
				actions = get_all_keybind_actions();
				if (creation_mode)
				{
					keybind_name = "Keybind";
					selected_action = 0;
					keybind.key_code = -1;
					keybind.action = actions[selected_action];
				}
			}

			std::vector<std::string> action_names;
			std::vector<const char*> action_names_cstr;

			for (const auto& action : actions)
			{
				action_names_cstr.push_back(action->name().c_str());
			}

			std::string name_id = "##Input" + std::string(id);
			if (show_name_field)
			{
				ImGui::Text("Keybind Name");
				ImGui::InputText(name_id.c_str(), &keybind_name);
			}
			
			if (show_keybind_field)
			{
				std::string key_input_id = name_id + "Key";
				ImGui::Text("Press the desired key combination");
				ImGui::BeginDisabled();
				std::string key_combination = keybind.name();
				ImGui::InputText(key_input_id.c_str(), &key_combination, ImGuiInputTextFlags_ReadOnly);
				ImGui::EndDisabled();
			}

			if (show_action_field)
			{
				ImGui::Text("Action");
				std::string combo_id = "##Combo" + std::string(id);
				if (ImGui::Combo(combo_id.c_str(), &selected_action, action_names_cstr.data(), static_cast<int>(actions.size())))
				{
					keybind.action = actions[selected_action];
				}
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
				if (ImGui::BeginTable("##KeybindPropertiesBackground", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
				{
					keybind.action->render_properties();
					ImGui::EndTable();
				}
				ImGui::PopStyleColor();
			}
			ImGui::Dummy(style.ItemSpacing);
			bool is_valid = std::invoke(validator, keybind_name, keybind, validator_mode);
			if (!is_valid) ImGui::BeginDisabled();
			if (ImGui::Button(!show_save_button ? "Add" : "Save"))
			{
				result = (keybind.key_code != -1);
				ImGui::CloseCurrentPopup();
			}
			if (!is_valid) ImGui::EndDisabled();
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
