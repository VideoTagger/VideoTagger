#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace vt::widgets
{
	extern bool checkbox(const char* label, bool* value);
	extern bool icon_button(const char* label, const ImVec2& size = ImVec2(0, 0));
	extern bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size = ImVec2(0, 0));

	extern bool collapsing_header(const char* label, bool hide_background = false);
	extern void label(const char* label);

	bool begin_button_dropdown(const char* label, ImVec2 button_size, float popup_height = 0.0f);
	void end_button_dropdown();

	void help_marker(const char* description);
}
