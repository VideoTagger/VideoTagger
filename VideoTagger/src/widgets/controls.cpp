#include "controls.hpp"
#include "icons.hpp"
#include <string>

#include <imgui_internal.h>

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

	//Original Author: https://github.com/ocornut/imgui/issues/474#issuecomment-169480920
	//This is a modified version
	bool begin_button_dropdown(const char* label, ImVec2 button_size, float popup_height)
	{
		ImGui::SameLine(0.f, 0.f);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		auto& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImVec2 cursor_pos = ImGui::GetCursorPos();

		ImVec2 size(20, button_size.y);
		bool pressed = ImGui::Button("##", size);

		// Arrow
		ImVec2 center(window->Pos.x + cursor_pos.x + 10, window->Pos.y + cursor_pos.y + button_size.y / 2);
		float r = 8.f;
		center.y -= r * 0.25f;
		ImVec2 a = center + ImVec2(0, 1) * r;
		ImVec2 b = center + ImVec2(-0.866f, -0.5f) * r;
		ImVec2 c = center + ImVec2(0.866f, -0.5f) * r;

		window->DrawList->AddTriangleFilled(a, b, c, ImGui::GetColorU32(ImGuiCol_Text));

		// Popup

		ImVec2 popup_pos;

		popup_pos.x = window->Pos.x + cursor_pos.x - button_size.x;
		popup_pos.y = window->Pos.y + cursor_pos.y + button_size.y;
		auto viewport = ImGui::GetWindowViewport();
		auto end_height = popup_pos.y + popup_height;
		if (popup_height != 0.0f and end_height > viewport->Size.y)
		{
			popup_pos.y = window->Pos.y + cursor_pos.y - popup_height;
		}
		ImGui::SetNextWindowPos(popup_pos);

		if (pressed)
		{
			ImGui::OpenPopup(label);
		}

		if (ImGui::BeginPopup(label))
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_Button]);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Button]);
			return true;
		}

		return false;
	}

	void end_button_dropdown()
	{
		ImGui::PopStyleColor(3);
		ImGui::EndPopup();
	}

	void help_marker(const char* description)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::TextDisabled(icons::help);
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(description);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();
	}
}
