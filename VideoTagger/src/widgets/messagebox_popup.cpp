#include "pch.hpp"
#include "messagebox_popup.hpp"

namespace vt::widgets
{
	messagebox_result messagebox_popup(const std::string& title, const std::string& description, const std::vector<std::string>& buttons, size_t default_button)
	{
		messagebox_result result;
		const char* title_cstr = title.c_str();
		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		static constexpr auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
		bool popup_open = ImGui::BeginPopupModal(title_cstr, nullptr, flags);
		ImGui::PopStyleVar(2);

		if (popup_open)
		{
			const char* description_cstr = description.c_str();
			ImGui::TextUnformatted(description_cstr);

			size_t button_count = buttons.size();
			if (button_count > 0)
			{
				float buttons_row_width = 2 * style.ItemSpacing.x * (button_count - 1);

				for (size_t i = 0; i < button_count; ++i)
				{
					buttons_row_width += ImGui::CalcTextSize(buttons[i].c_str()).x;
				}

				auto avail_area = ImGui::CalcTextSize(description_cstr).x;

				ImGui::SetCursorPosX(avail_area / 2 - buttons_row_width / 2);
				for (size_t i = 0; i < button_count; ++i)
				{
					if (ImGui::Button(buttons[i].c_str()))
					{
						result.index = i;
						ImGui::CloseCurrentPopup();
					}

					if (i < button_count - 1)
					{
						ImGui::SameLine();
					}
				}
			}

			if (ImGui::IsKeyDown(ImGuiKey_Enter))
			{
				result.index = default_button;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::IsKeyDown(ImGuiKey_Escape))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return result;
	}
}
