#pragma once
#include <optional>
#include <memory>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <imgui.h>

#include "project.hpp"
#include "input.hpp"
#include "keybind_storage.hpp"
#include "theme.hpp"
#include <video/video_stream.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/color_picker.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/theme_customizer.hpp>
#include <widgets/modal/options.hpp>
#include "displayed_videos_manager.hpp"
#include <utils/json.hpp>

namespace vt
{
	enum class window_state : uint8_t
	{
		normal,
		minimized, //not serialized
		maximized
	};

	struct app_settings
	{
		float thumbnail_size = 72.0f;
		bool link_start_end_segment = true;
	};

	struct window_config
	{
		//serialized
		window_state state = window_state::normal;
		bool show_inspector_window = true;
		bool show_tag_manager_window = true;
		bool show_timeline_window = true;
		bool show_video_player_window = true;
		bool show_video_browser_window = true;
		bool show_video_group_browser_window = true;

		//not serialized
		bool show_options_window = false;
		bool show_theme_customizer_window = false;
		bool show_about_window = false;
	};

	struct app_context
	{
		widgets::timeline_state timeline_state;
		widgets::project_selector project_selector;
		widgets::video_player player;
		widgets::video_browser browser;
		widgets::video_group_browser group_browser;
		widgets::theme_customizer theme_customizer;
		widgets::modal::options options;
		widgets::color_picker color_picker;

		std::filesystem::path projects_list_filepath = std::filesystem::path("projects").replace_extension("json");
		std::filesystem::path app_settings_filepath = std::filesystem::path("settings").replace_extension("json");
		std::filesystem::path theme_dir_filepath = "themes";
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::unordered_map<std::string, ImFont*> fonts;
		std::vector<std::filesystem::path> themes;
		keybind_storage keybinds;

		std::optional<project> current_project;
		video_group_id_t current_video_group_id{};
		displayed_videos_manager displayed_videos;

		std::optional<widgets::selected_segment_data> selected_segment_data;
		std::optional<widgets::moving_segment_data> moving_segment_data;

		app_settings app_settings;
		SDL_Window* main_window{};
		SDL_Renderer* renderer{};

		bool is_project_dirty{};
		bool first_launch = true;
		bool reset_layout{};
		bool reset_player_docking{};

		void update_current_video_group();
		void reset_current_video_group();

		segment_storage& get_current_segment_storage();
	};

	inline app_context ctx_;
}
