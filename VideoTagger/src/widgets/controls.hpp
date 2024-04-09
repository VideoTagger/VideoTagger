#pragma once
#include <string>
#include <functional>

#include <SDL.h>
#include <imgui.h>
#include <utils/timestamp.hpp>

namespace vt::widgets
{
	extern bool checkbox(const char* label, bool* value);
	extern bool icon_button(const char* label, const ImVec2& size = ImVec2(0, 0));
	extern bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size = ImVec2(0, 0));

	extern bool collapsing_header(const char* label, bool hide_background = false);
	extern void label(const char* label);

	extern bool begin_button_dropdown(const char* label, ImVec2 button_size, float popup_height = 0.0f);
	extern void end_button_dropdown();

	extern void help_marker(const char* description);
	extern void centered_text(const char* text, ImVec2 avail_area);
	extern void clipped_text(const char* text, ImVec2 avail_area);

	extern bool timestamp_control(const std::string& name, timestamp& timestamp, uint64_t min_timestamp, uint64_t max_timestamp, bool* was_activated, bool* was_released, bool fill_area = true);
	
	extern bool tile(const std::string& label, ImVec2 tile_size, ImVec2 image_size, SDL_Texture* image, const std::function<void(const std::string&)> context_menu = nullptr, const std::function<void(const std::string&)> drag_drop = nullptr);
}
