#pragma once
#include <optional>
#include <memory>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>

#include <imgui.h>

#include "project.hpp"
#include "input.hpp"
#include "keybind_storage.hpp"
#include "theme.hpp"
#include "eng_lang_pack.hpp"

#include "main_window.hpp"

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
#include <widgets/modal/script_progress.hpp>
#include "displayed_videos_manager.hpp"
#include <utils/json.hpp>
#include <scripts/scripting_engine.hpp>
#include <services/service_account_manager.hpp>
#include <video/video_importer.hpp>

#include <editor/registry.hpp>

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
		shutdown
	};

	struct app_settings
	{
		float thumbnail_size = 45.0f;
		bool link_start_end_segment = true;
		bool autoplay = true;
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
		bool show_script_progress = false;
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
		widgets::modal::script_progress script_progress;
		widgets::color_picker color_picker;
		widgets::modal::tag_importer tag_importer;

		static constexpr auto valid_video_extensions = std::array{ "mp4", "mkv", "avi", "mov", "flv", "wmv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "vob", "mts", "m2ts", "mxf", "f4v", "divx", "rmvb", "asf", "swf" };

		std::filesystem::path projects_list_filepath = std::filesystem::path("projects").replace_extension("json");
		std::filesystem::path app_settings_filepath = std::filesystem::path("settings").replace_extension("json");
		std::filesystem::path accounts_filepath = std::filesystem::path("accounts").replace_extension("json");
		std::filesystem::path scripts_filepath = std::filesystem::path("assets") / "scripts";
		std::filesystem::path theme_dir_filepath = "themes";
		registry registry;
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::unordered_map<std::string, ImFont*> fonts;
		std::vector<std::filesystem::path> themes;
		keybind_storage keybinds;
		scripting_engine script_eng;
		std::optional<script_handle> script_handle;
		std::unordered_map<std::string, std::unique_ptr<service_account_manager>> account_managers;
		std::unordered_map<std::string, std::unique_ptr<video_importer>> video_importers;

		displayed_videos_manager displayed_videos;

		widgets::insert_segment_data_container insert_segment_data;

		app_settings app_settings;
		lang_pack<lang_pack_id> lang{ eng_lang_data };
		std::unique_ptr<main_window> main_window{};

		app_state state_ = app_state::uninitialized;

		bool is_project_dirty{};
		bool first_launch = true;
		bool reset_layout{};
		bool reset_player_docking{};

		bool pause_player = false;

		//TODO: add get and is registered for account managers
		template<typename ServiceAccountManager>
		void register_account_manager();
		void register_account_managers();

		template<typename VideoImporter>
		void register_video_importer();
		void register_video_importers();
		template<typename VideoImporter>
		VideoImporter& get_video_importer();
		video_importer& get_video_importer(const std::string& importer_id);
		template<typename VideoImporter>
		bool is_video_importer_registered() const;
		bool is_video_importer_registered(const std::string& importer_id) const;

		void register_handlers();
		void update_current_video_group();
		void reset_current_video_group();

		segment_storage& get_current_segment_storage();
	
		void set_current_video_group_id(video_group_id_t id);
		video_group_id_t current_video_group_id() const;

	private:
		video_group_id_t current_video_group_id_{};
	};

	inline app_context ctx_;

	template<typename ServiceAccountManager>
	inline void app_context::register_account_manager()
	{
		//TODO: Check if is registered

		account_managers[ServiceAccountManager::static_service_name] = std::make_unique<ServiceAccountManager>();
	}

	template<typename VideoImporter>
	inline void app_context::register_video_importer()
	{
		if (is_video_importer_registered<VideoImporter>())
		{
			debug::error("Video importer with id {} is already registered", VideoImporter::static_importer_id);
			return;
		}

		video_importers[VideoImporter::static_importer_id] = std::make_unique<VideoImporter>();
	}

	template<typename VideoImporter>
	inline VideoImporter& app_context::get_video_importer()
	{
		VideoImporter* result = dynamic_cast<VideoImporter>(video_importers.at(VideoImporter::static_importer_id).get());
		if (result == nullptr)
		{
			debug::panic("Video importer type in the template argument didn't match the registered type for id {}", VideoImporter::static_importer_id);
		}

		return *result;
	}

	template<typename VideoImporter>
	inline bool app_context::is_video_importer_registered() const
	{
		return video_importers.count(VideoImporter::static_importer_id) != 0;
	}
}
