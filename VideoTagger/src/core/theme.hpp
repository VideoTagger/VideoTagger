#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include <stack>
#include <unordered_map>
#include <imgui.h>

namespace vt
{
	enum class theme_color
	{
		accent_light,
		accent_medium,
		accent_dark,
		accent_background,
		secondary_light,
		secondary_medium,
		secondary_dark,
		selection_normal,
		selection_disabled,
		playhead_normal,
		playhead_disabled,
		axis_x,
		axis_y,
		axis_z,
		icon_thumbnail,
		common_success,
		common_info,
		common_warning,
		common_error,
		console_info,
		console_warning,
		console_error,
		button_normal,
		button_hover,
		button_active,
		text_normal,
		text_inverted,
		text_disabled,
		background_window,
		background_child,
		background_popup,
		background_base,
		background_base_alt,
		background_secondary,
		background_tertiary,
		border,
		frame_background_normal,
		frame_background_hover,
		frame_background_active,
		title_background_normal,
		title_background_active,
		title_background_collapsed,
		menubar_background,
		scrollbar_background,
		scrollbar_grab_normal,
		scrollbar_grab_hover,
		scrollbar_grab_active,
		checkmark,
		header_normal,
		header_hover,
		header_active,
		separator_normal,
		separator_hover,
		separator_active,
		tab_focused_normal,
		tab_focused_hover,
		tab_focused_active,
		tab_unfocused_normal,
		tab_unfocused_active,
	};

	struct color_override
	{
		theme_color color;
		uint32_t previous_value;
	};

	struct theme
	{
	public:
		static constexpr const char* extension = "vttheme";

		theme() = default;
		theme(const theme&) = default;

	private:
		std::unordered_map<theme_color, uint32_t> colors_;
		std::stack<color_override> color_overrides_;
		std::string name_;
		bool is_dark_ = true;

	public:
		void apply();
		void push_color(theme_color color, uint32_t rgba);
		void push_color(theme_color color, ImVec4 rgba);
		void pop_color(size_t count = 1);

		void set_name(const std::string& name);
		void set_dark(bool is_dark);

		void set_color(theme_color name, uint32_t rgba);
		void set_color(theme_color name, ImVec4 color);

		const std::string& name() const;
		bool is_dark() const;

		uint32_t get_rgba(theme_color name) const;
		uint32_t get_rgba(const std::string& name) const;
		ImVec4 get_float4(theme_color name) const;
		ImVec4 get_float4(const std::string& name) const;

		void save(const std::filesystem::path& filepath) const;
		static theme load_from_file(const std::filesystem::path& filepath);

		static std::optional<theme_color> to_theme_color(const std::string& name);
		static std::string to_string(theme_color color);
	};
}
