#pragma once
#include <optional>
#include <memory>
#include "project.hpp"

#include <imgui.h>
#include <json.hpp>
#include <video/video.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/color_picker.hpp>

namespace vt
{
	enum class window_state : uint8_t
	{
		normal,
		minimized, //not serialized
		maximized
	};

	struct window_config
	{
		//serialized
		window_state state = window_state::normal;
		bool show_inspector_window = true;
		bool show_settings_window = true;
		bool show_tag_manager_window = true;
		bool show_video_player_window = true;

		//not serialized
		bool show_about_window = false;
	};

	struct app_context
	{
		widgets::project_selector project_selector;
		widgets::video_player player;
		std::optional<project> current_project;
		widgets::color_picker color_picker;
		std::filesystem::path projects_list_filepath;
		std::filesystem::path app_settings_filepath;
		std::vector<std::shared_ptr<video>> videos;
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::unordered_map<std::string, ImFont*> fonts;
		std::optional<widgets::selected_timestamp_data> selected_timestamp_data;
		bool is_project_dirty{};
		bool reset_layout{};
	};

	inline app_context ctx_;
}
