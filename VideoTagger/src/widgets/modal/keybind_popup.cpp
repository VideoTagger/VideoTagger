#include "pch.hpp"
#include "keybind_popup.hpp"
#include <core/debug.hpp>

namespace vt::widgets::modal
{
	bool keybind_popup(const char* id, const keybind& keybind, const vt::keybind& last_keybind, const std::function<bool(const std::string&, const vt::keybind&, keybind_validator_mode)>& validator)
	{
		if (validator == nullptr) debug::panic("Keybind validator was nullptr");

		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (ImGui::BeginPopupModal(id, nullptr, flags))
		{
			ImGui::Text("Press the desired key combination and then press Enter");
			std::string input_id = "##Input" + std::string(id);
			std::string key_combination = last_keybind.name();
			ImGui::BeginDisabled();
			ImGui::InputText(input_id.c_str(), &key_combination, ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				result = std::invoke(validator, "", last_keybind, keybind_validator_mode::validate_keybind) and (last_keybind.key_code != -1);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);
		return result;
	}
}
