#pragma once
#include <system/system_window.hpp>
#include <utils/file_node.hpp>
#include <events/event_source.hpp>

namespace vt
{
	class main_window : public system_window
	{
	public:
		main_window(const system_window_config& cfg);

	private:
		event_source event_source_;

	public:
		void register_listeners();

		void on_close_project(bool should_shutdown);
		void on_save();
		void on_save_as();
		void on_dont_save();
		void on_show_in_explorer();
		void on_import_videos();
		void on_delete();
		void on_launch();
		void on_shutdown();
		void on_first_launch();

		bool load_accounts();
		bool load_settings();
		void load_theme();
		void save_settings();
		void save_project();
		void save_project_as(const std::filesystem::path& filepath);
		void close_project();

		void copy_app_assets();

		void init_keybinds();
		void init_player();
		void init_options();

		utils::file_node fetch_themes(const std::filesystem::path& path);
		utils::file_node fetch_scripts(const std::filesystem::path& path);

		void draw_menubar();
		void draw_project_selector();
		void draw_main_app();

		void draw_video_widgets();

		void enable_undocking(bool value);

		virtual void on_render() override;
		virtual void handle_event(const SDL_Event& event) override;
	};
}
