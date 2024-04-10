#include "pch.hpp"
#include "create_group_popup.hpp"

namespace vt::widgets::modal
{
	bool create_group_popup(const std::string& id, std::string& group_name)
	{
		bool result{};

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool is_open = ImGui::BeginPopupModal(id.c_str(), nullptr, flags);
		ImGui::PopStyleVar(2);

		bool is_appearing = ImGui::IsWindowAppearing();

		//TODO: This should be temporary, remove when this becomes a class
		if (is_appearing)
		{
			group_name.clear();
		}

		if (is_open)
		{
			std::string name_id = "##Input" + std::string(id);
			
			ImGui::Text("Group Name");

			if (is_appearing)
			{
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::InputText(name_id.c_str(), &group_name);

			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("Create"))
			{
				result = true;
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
