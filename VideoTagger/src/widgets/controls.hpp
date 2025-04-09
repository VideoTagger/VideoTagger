#pragma once
#include <string>
#include <functional>

#include <SDL.h>
#include <imgui.h>
#include <utils/timestamp.hpp>
#include <utils/vec.hpp>

namespace vt::widgets
{
	extern bool checkbox(const char* label, bool* value);
	extern bool icon_button(const char* label, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	extern bool icon_button_no_cursor(const char* label, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));
	extern void tooltip(const char* text);
	extern bool icon_toggle_button(const char* label, bool is_toggled, const ImVec2& size = ImVec2{}, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_Text));

	extern bool collapsing_header(const char* label, bool hide_background = false);
	extern void label(const char* label);

	extern bool begin_button_dropdown(const char* label, ImVec2 button_size, float popup_height = 0.0f);
	extern void end_button_dropdown();

	extern void help_marker(const char* description);
	extern void centered_text(const char* text, ImVec2 avail_area, ImVec2 offset = {});
	extern void clipped_text(const char* text, ImVec2 avail_area);
	extern void text_with_size(const char* text, ImVec2 size = { 0.f, 0.f });

	extern bool timestamp_control(const std::string& name, timestamp& timestamp, uint64_t min_timestamp, uint64_t max_timestamp, bool* was_activated, bool* was_released, bool fill_area = true);
	extern bool frame_dragger(int64_t& frame, bool& is_dragging);
	
	extern bool search_bar(const char* label, const char* hint, std::string& buffer, float width = 0.0f, bool enable_button = true, ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_EnterReturnsTrue);

	extern ImVec2 calc_selectable_tile_size(ImVec2 tile_size);
	//TODO: Consider moving these parameters into a struct
	extern bool tile(
		const char* id, const std::string& label, ImVec2 tile_size, ImVec2 image_size, GLuint image,
		const std::function<void(const std::string& /*label*/)> context_menu = nullptr,
		const std::function<void(const std::string& /*label*/)> drag_drop = nullptr,
		std::function<void(ImDrawList& /*label*/, ImRect /*item rect*/, ImRect /*image rect*/)> custom_draw = nullptr,
		ImVec2 uv0 = {0, 0}, ImVec2 uv1 = {1, 1}, bool is_selected = false
	);

	extern bool selection_area(ImVec2& start_pos, ImVec2& end_pos, ImGuiMouseButton mouse_button = ImGuiMouseButton_Left);

	extern void color_indicator(float thickness, uint32_t color);
	extern bool begin_collapsible(const std::string& id, const std::string& label, ImGuiTreeNodeFlags flags = 0, const char* icon = nullptr, const std::optional<ImVec4>& icon_color = std::nullopt, const std::function<void(void)>& on_dragdrop = nullptr, const std::optional<size_t>& index = std::nullopt);
	extern void end_collapsible();

	extern bool is_item_disabled();
	extern bool table_hovered_row_style();

	extern bool positon_control(utils::vec2<uint32_t>& pos, const utils::vec2<uint32_t>& max_size);
}
