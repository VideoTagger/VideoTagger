#pragma once
#include <pch.hpp>
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
#include "localization/lang_pack.hpp"

#include "font_type.hpp"
#include "main_window.hpp"
#include "app_settings.hpp"

#include <widgets/project_selector.hpp>
#include <widgets/color_picker.hpp>
#include <ui/popups/options_popup.hpp>
#include <widgets/modal/tag_importer.hpp>
#include "displayed_videos_manager.hpp"
#include <utils/json.hpp>
#include <utils/vec.hpp>
#include <utils/file_node.hpp>
#include <scripts/scripting_engine.hpp>
#include <services/service_account_manager.hpp>
#include <video/video_importer.hpp>
#include <ui/popups/segments_move_conflict_popup.hpp>
#include <ui/popups/segment_insert_conflict_popup.hpp>
#include <ui/popups/segment_insert_popup.hpp>
#include <ui/popups/tag_rename_failed_popup.hpp>
#include <ui/popups/script_progress_popup.hpp>
#include <ui/popups/messagebox_popup.hpp>

#include <ui/ui_registry.hpp>
#include <events/event_storage.hpp>
#include <tasks/task_manager.hpp>
#include "session_storage.hpp"
#include <attributes/attribute_registry.hpp>
#include <attributes/shape_predictor_registry.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/shapes/line_shape.hpp>
#include <attributes/shapes/points_shape.hpp>
#include <attributes/shapes/polygon_shape.hpp>
#include <attributes/shapes/circle_shape.hpp>

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

	struct window_config
	{
		//serialized
		window_state state = window_state::normal;

		//not serialized
		bool show_options_window = false;
		bool show_about_window = false;
		bool show_tag_importer_window = false;
	};

	using shape_predictor_registries_type = std::tuple<
		shape_predictor_registry<rectangle_shape>,
		shape_predictor_registry<line_shape>,
		shape_predictor_registry<points_shape>,
		shape_predictor_registry<polygon_shape>,
		shape_predictor_registry<circle_shape>
	>;


	///@brief Application context that holds all states and necessary data
	struct app_context : public event_storage, ui::ui_registry
	{
		static constexpr auto valid_video_extensions = std::array{ "mp4", "mkv", "avi", "mov", "flv", "wmv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "vob", "mts", "m2ts", "mxf", "f4v", "divx", "rmvb", "asf", "swf" };
		app_context();

		task_manager tasks;

		std::optional<project> current_project;
		widgets::project_selector project_selector;
		ui::options_popup options{ &win_cfg.show_options_window };
		widgets::color_picker color_picker;
		widgets::modal::tag_importer tag_importer;
		ui::messagebox_popup messagebox;

		//TODO: maybe add some popup manager
		std::unique_ptr<ui::script_progress_popup> script_progress_popup;
		std::unique_ptr<ui::segments_move_conflict_popup> segments_move_conflict_popup;
		std::unique_ptr<ui::segment_insert_conflict_popup> segment_insert_conflict_popup;
		std::unique_ptr<ui::segment_insert_popup> segment_insert_popup;
		std::unique_ptr<ui::tag_rename_failed_popup> tag_rename_failed_popup;

		std::filesystem::path assets_dir_filepath = std::filesystem::path("assets");
		std::filesystem::path projects_list_filepath = storage_path() / std::filesystem::path("projects").replace_extension("json");
		std::filesystem::path app_settings_filepath = storage_path() / std::filesystem::path("settings").replace_extension("json");
		std::filesystem::path accounts_filepath = storage_path() / std::filesystem::path("accounts").replace_extension("json");
		std::filesystem::path script_dir_filepath = assets_dir_filepath / "scripts";
		std::filesystem::path lang_dir_filepath = storage_path() / "lang";
		std::filesystem::path theme_dir_filepath = assets_dir_filepath / "themes"; //storage_path() / "themes";
		std::filesystem::path downloads_dir_filepath = storage_path() / "downloads";
		std::filesystem::path cache_dir_filepath = storage_path() / std::filesystem::path("cache");
		std::filesystem::path thumbnail_dir_filepath = cache_dir_filepath / "thumbnails";
		std::filesystem::path python_dir_filepath = assets_dir_filepath / "python";
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::unordered_map<font_type, ImFont*> fonts;
		theme current_theme;
		utils::file_node scripts;
		utils::file_node themes;
		keybind_storage keybinds;
		scripting_engine script_eng;
		std::optional<script_handle> script_handle;
		std::unordered_map<std::string, std::unique_ptr<service_account_manager>> account_managers;
		std::unordered_map<std::string, std::unique_ptr<video_importer>> video_importers;
		std::optional<video_id_t> last_focused_video;

		attribute_registry attr_registry;
		shape_predictor_registries_type shape_predictor_registries;

		session_storage session;

		displayed_videos_manager displayed_videos;

		app_settings app_settings;
		std::shared_ptr<lang_pack> lang = nullptr;
		std::vector<std::shared_ptr<lang_pack>> lang_packs;
		std::unique_ptr<main_window> main_window{};

		app_state state_ = app_state::uninitialized;

		bool is_project_dirty{};
		bool first_launch = true;
		bool reset_layout{};
		bool reset_player_docking{};

		void init_attribute_registry();
		void init_shape_predictor_registries();

		void create_windows();
		void create_popups();
		void render_messagebox();

		void change_theme(const theme& new_theme);
		[[nodiscard]] nlohmann::ordered_json serialize_app_settings();
		void deserialize_app_settings(const nlohmann::ordered_json& json);
		
		template<typename service_account_manager_type>
		void register_account_manager();
		void register_account_managers();
		template<typename service_account_manager_type>
		service_account_manager_type& get_account_manager();
		service_account_manager& get_account_manager(const std::string& service_id);
		template<typename service_account_manager_type>
		bool is_account_manager_registered() const;
		bool is_account_manager_registered(const std::string& service_id) const;

		template<typename video_importer_type>
		void register_video_importer();
		void register_video_importers();
		template<typename video_importer_type>
		video_importer_type& get_video_importer();
		video_importer& get_video_importer(const std::string& importer_id);
		template<typename video_importer_type>
		bool is_video_importer_registered() const;
		bool is_video_importer_registered(const std::string& importer_id) const;

		void update_current_video_group();

		segment_storage& get_current_segment_storage();

		std::shared_ptr<lang_pack> load_lang_pack(const std::string& name = "en_US");
		std::shared_ptr<lang_pack> load_or_create_lang_pack(const std::string& name, const std::string& filename);
		void insert_lang_pack(std::shared_ptr<lang_pack> pack);
		void remove_lang_pack(const std::string& name);
		void load_lang_packs(const std::string& desired_lang);
		std::vector<std::string> lang_names() const;

		void run_script(const std::filesystem::path& script_path);

		ImFont* get_font(font_type type = font_type::normal) const;
		std::optional<utils::vec2<int>> get_active_video_tex_size() const;

		static std::filesystem::path storage_path();

		template<typename shape_type>
		shape_predictor_registry<shape_type>& get_shape_predictor_registry();
	};

	///@brief Global application context instance
	inline app_context ctx_;

	template<typename service_account_manager_type>
	inline void app_context::register_account_manager()
	{
		if (is_account_manager_registered<service_account_manager_type>())
		{
			debug::error("Account manager with id {} is already registered", service_account_manager_type::static_service_id);
			return;
		}

		account_managers[service_account_manager_type::static_service_id] = std::make_unique<service_account_manager_type>();
	}

	template<typename service_account_manager_type>
	inline service_account_manager_type& app_context::get_account_manager()
	{
		service_account_manager_type* result = dynamic_cast<service_account_manager_type*>(account_managers.at(service_account_manager_type::static_service_id).get());
		if (result == nullptr)
		{
			debug::panic("Account manager type in the template argument didn't match the registered type for id {}", service_account_manager_type::static_service_id);
		}

		return *result;
	}

	template<typename service_account_manager_type>
	inline bool app_context::is_account_manager_registered() const
	{
		return account_managers.count(service_account_manager_type::static_service_id) != 0;
	}

	template<typename video_importer_type>
	inline void app_context::register_video_importer()
	{
		if (is_video_importer_registered<video_importer_type>())
		{
			debug::error("Video importer with id {} is already registered", video_importer_type::static_importer_id);
			return;
		}

		video_importers[video_importer_type::static_importer_id] = std::make_unique<video_importer_type>();
	}

	template<typename video_importer_type>
	inline video_importer_type& app_context::get_video_importer()
	{
		video_importer_type* result = dynamic_cast<video_importer_type*>(video_importers.at(video_importer_type::static_importer_id).get());
		if (result == nullptr)
		{
			debug::panic("Video importer type in the template argument didn't match the registered type for id {}", video_importer_type::static_importer_id);
		}

		return *result;
	}

	template<typename video_importer_type>
	inline bool app_context::is_video_importer_registered() const
	{
		return video_importers.count(video_importer_type::static_importer_id) != 0;
	}

	template<typename shape_type>
	inline shape_predictor_registry<shape_type>& app_context::get_shape_predictor_registry()
	{
		return std::get<shape_predictor_registry<shape_type>>(shape_predictor_registries);
	}
}
