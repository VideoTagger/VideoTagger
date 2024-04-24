#include <imgui.h>
#include <string>
#include <vector>
#include "message_box.hpp"


namespace vt::widgets
{
	messagebox_result message_box(const std::string& title, std::string& description, const std::vector<std::string>& buttons, bool is_modal, size_t default_button)
	{

		messagebox_result result;

		ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);

		if (is_modal) {
			if (!ImGui::IsPopupOpen(title.c_str())) 
			{
				ImGui::OpenPopup(title.c_str());
			}
			if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) 
			{
				ImGui::Text("%s", description.c_str());

				for (size_t i = 0; i < buttons.size(); ++i) 
				{
					if (ImGui::Button(buttons[i].c_str())) 
					{
						result.index = i;
						ImGui::CloseCurrentPopup();
					}
					if (i == default_button) 
					{
						ImGui::SetItemDefaultFocus();
					}
					if (i < buttons.size() - 1) 
					{
						ImGui::SameLine();
					}
				}
				ImGui::EndPopup();
			}
		}
		else {
			if (ImGui::BeginPopup(title.c_str())) 
			{
				ImGui::Text("%s", description.c_str());

				for (size_t i = 0; i < buttons.size(); ++i) 
				{
					if (ImGui::Button(buttons[i].c_str())) 
					{
						result.index = i;
					}
					if (i == default_button) 
					{
						ImGui::SetItemDefaultFocus();
					}
					if (i < buttons.size() - 1) 
					{
						ImGui::SameLine();
					}
				}
				ImGui::EndPopup();

			}
			
 
		}
		if (ImGui::IsKeyDown(ImGuiKey_Enter))
		{
			result.index = default_button;
		}

		if (ImGui::IsKeyDown(ImGuiKey_Escape)) 
		{
			 result.index = -1;
		}
		return result;
	}
