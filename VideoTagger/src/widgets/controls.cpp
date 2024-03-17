#include "controls.hpp"
#include "icons.hpp"
#include <string>

namespace vt::widgets
{
	bool checkbox(const char* label, bool* value)
	{
		auto& style = ImGui::GetStyle();

		bool result{};
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
		result = ImGui::Checkbox(label, value);
		ImGui::PopStyleVar();
		return result;
	}

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

	bool collapsing_header(const char* label, bool hide_background)
	{
		auto& style = ImGui::GetStyle();
		if (hide_background) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		std::string node_id = "##Node" + std::string(label);
		auto cx = ImGui::GetCursorPosX();
		bool result = ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		
		int pop_count = 1;
		if (hide_background) ++pop_count;
		ImGui::PopStyleColor(pop_count);
		auto icon = result ? icons::expand_less : icons::expand_more;

		ImGui::SameLine();
		auto px = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(px - (px - cx) + style.ItemInnerSpacing.x);
		//ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::Text(label);
		ImGui::SameLine(ImGui::GetContentRegionMax().x - style.FramePadding.x - ImGui::CalcTextSize(icon).x);
		ImGui::Text(icon);
		return result;
	}

	void label(const char* label)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, ImGui::GetStyle().FramePadding.y });
		ImGui::Button(label);
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
	}
}
