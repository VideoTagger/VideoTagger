#pragma once
#include <string>
#include <functional>
#include <imgui.h>

namespace vt::ui
{
	///@defgroup ux UX
	///@{
	void tooltip(const std::string& text);
	void tooltip(const std::string& text, ImVec2 pos);
	void help_marker(const std::string& description);
	///@}
	
	///@defgroup ui_styling UI Styling
	///@{
	///@return True if the scrollbar is hovered, false otherwise.
	bool is_scrollbar_hovered();
	void begin_styled_scrollbars(const std::string& window_id);
	void save_window_scrollbar_state(const std::string& window_id);
	void end_styled_scrollbars();
	void begin_bigger_frames();
	void end_bigger_frames();
	void begin_modal_style();
	void end_modal_style();
	void begin_rounded_window_style();
	void end_rounded_window_style();
	///@}
	
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
	bool icon_toggle_button(const std::string& label_on, const std::string& label_off, bool is_toggled, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	bool accent_button(const std::string& label, const ImVec2& size = ImVec2{});
	bool button(const std::string& label, const ImVec2& size = ImVec2{});
	bool color_button(const std::string& id, const ImVec4& color, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
	bool color_edit3(const std::string& label, ImVec4& color, ImGuiColorEditFlags flags = 0);
	bool color_edit4(const std::string& label, ImVec4& color, ImGuiColorEditFlags flags = 0);

	bool toggle(const std::string& label, bool& value);
	bool checkbox(const std::string& label, bool& value);
	///@}

	///@defgroup ui_components UI Components
	///@{
	bool collapsing_header(const std::string& label, bool hide_background = false);
	bool card(const std::function<void()>& body, bool border = false);
	///@}

	///@defgroup ui_menu UI Menus
	///@{
	bool begin_main_menu(const std::string& label, bool enabled = true);
	bool begin_menu(const std::string& label, bool enabled = true);
	void end_menu();
	///@}

	float table_border_size();
	float toggle_height();
	ImVec2 bigger_frame_size();
	bool is_item_disabled();
}
