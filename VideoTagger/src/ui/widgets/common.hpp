#pragma once
#include <string>
#include <imgui.h>

namespace vt::ui
{
	void tooltip(const std::string& text);
	void help_marker(const std::string& description);
	
	///@defgroup ui_text UI Text
	///@{
	void label(const std::string& label);	
	void centered_text(const std::string& text, ImVec2 avail_area, ImVec2 offset = {});
	void clipped_text(const std::string& text, ImVec2 avail_area);
	void text_with_size(const std::string& text, ImVec2 size = {});
	///@}


	///@defgroup ui_layout UI Layout
	///@{
	void item_spacer(const ImVec2& size = {});
	void vertical_item_spacer(float height = 0.f);
	void horizontal_item_spacer(float width = 0.f);
	///@}

	///@defgroup ui_buttons UI Buttons
	///@{
	bool rounded_button(const std::string& label, const ImVec2& size = ImVec2{});
	bool icon_button(const std::string& label, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	bool icon_button_no_cursor(const std::string& label, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	bool icon_toggle_button(const std::string& label, bool is_toggled, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	bool accent_button(const std::string& label, const ImVec2& size = ImVec2{});
	bool button(const std::string& label, const ImVec2& size = ImVec2{});

	bool toggle(const std::string& label, bool& value);
	bool checkbox(const std::string& label, bool& value);
	///@}

	bool collapsing_header(const std::string& label, bool hide_background = false);

	///@defgroup ui_menu UI Menus
	///@{
	bool begin_main_menu(const std::string& label, bool enabled = true);
	bool begin_menu(const std::string& label, bool enabled = true);
	void end_menu();
	///@}

	float toggle_height();
	bool is_item_disabled();
}
