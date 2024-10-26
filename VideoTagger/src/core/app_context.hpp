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
#include "eng_lang_pack.hpp"

#include "main_window.hpp"
#include "tool_window.hpp"

#include <video/video_stream.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/color_picker.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/video_group_queue.hpp>
#include <widgets/theme_customizer.hpp>
#include <widgets/modal/options.hpp>
#include <widgets/modal/tag_importer.hpp>
#include "displayed_videos_manager.hpp"
#include <utils/json.hpp>
#include <scripts/scripting_engine.hpp>

namespace vt
{
	enum class window_state : uint8_t
	{
		normal,
		minimized, //not serialized
		maximized
	};

	enum class app_state
	{
		uninitialized,
		initialized,
		running,
		script_running,
		shutdown
	};

	struct app_settings
	{
		float thumbnail_size = 45.0f;
		bool link_start_end_segment = true;
		bool next_video_on_end = true;
		bool load_thumbnails = true;
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
		bool show_video_group_queue_window = true;

		//not serialized
		bool show_options_window = false;
		bool show_theme_customizer_window = false;
		bool show_about_window = false;
		bool show_tag_importer_window = false;
	};

	struct app_context
	{
		std::optional<project> current_project;
		widgets::video_timeline video_timeline;
		widgets::project_selector project_selector;
		widgets::video_player player;
		widgets::video_browser browser;
		widgets::video_group_browser group_browser;
		widgets::video_group_queue group_queue;
		widgets::theme_customizer theme_customizer;
		widgets::modal::options options;
		widgets::color_picker color_picker;
		widgets::modal::tag_importer tag_importer;

		std::filesystem::path projects_list_filepath = std::filesystem::path("projects").replace_extension("json");
		std::filesystem::path app_settings_filepath = std::filesystem::path("settings").replace_extension("json");
		std::filesystem::path scripts_filepath = std::filesystem::path("assets") / "scripts";
		std::filesystem::path theme_dir_filepath = "themes";
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::unordered_map<std::string, ImFont*> fonts;
		std::vector<std::filesystem::path> themes;
		keybind_storage keybinds;
		scripting_engine script_eng;

		displayed_videos_manager displayed_videos;

		widgets::insert_segment_data_container insert_segment_data;

		app_settings app_settings;
		lang_pack<lang_pack_id> lang{ eng_lang_data };
		std::unique_ptr<main_window> main_window{};
		std::unique_ptr<tool_window> tool_window{};

		app_state state_ = app_state::uninitialized;

		bool is_project_dirty{};
		bool first_launch = true;
		bool reset_layout{};
		bool reset_player_docking{};

		bool pause_player = false;

		void update_current_video_group();
		void reset_current_video_group();

		segment_storage& get_current_segment_storage();
	
		void set_current_video_group_id(video_group_id_t id);
		video_group_id_t current_video_group_id() const;

	private:
		video_group_id_t current_video_group_id_{};
	};

	inline app_context ctx_;
}
