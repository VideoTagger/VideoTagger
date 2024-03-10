#include "buttons.hpp"

namespace vt::widgets
{
	bool icon_button(const char* label, const ImVec2& size)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();
		return result;
	}

	bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size)
	{
		if (!is_toggled) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		bool result = icon_button(label, size);
		if (!is_toggled) ImGui::PopStyleColor();
		return result;
	}
}
