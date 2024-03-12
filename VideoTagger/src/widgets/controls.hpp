#pragma once
#include <imgui.h>

namespace vt::widgets
{
	extern bool icon_button(const char* label, const ImVec2& size = ImVec2(0, 0));
	extern bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size = ImVec2(0, 0));

	extern bool collapsing_header(const char* label, bool hide_background = false);
	extern void label(const char* label);
}
