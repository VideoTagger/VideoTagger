#pragma once
#include <string>
#include <imgui.h>

namespace vt::ui
{
	bool rounded_button(const std::string& label, const ImVec2& size = ImVec2{});
	bool button(const std::string& label, const ImVec2& size = ImVec2{});
	float toggle_height();
	bool toggle(const std::string& label, bool& value);
	bool accent_button(const std::string& label, const ImVec2& size = ImVec2{});
	bool begin_main_menu(const std::string& label, bool enabled = true);
	bool begin_menu(const std::string& label, bool enabled = true);
	void end_menu();
}
