#pragma once
#include "app_window.hpp"
#include <utils/file_node.hpp>

namespace vt
{
	class main_window : public app_window
	{
	public:
		main_window(const app_window_config& cfg);

	public:
		bool on_close_project(bool should_shutdown);
		void on_save();
		void on_save_as();
		void on_show_in_explorer();
		void on_import_videos();
		void on_delete();

		bool load_accounts();
		bool load_settings();
		void save_settings();
		void save_project();
		void save_project_as(const std::filesystem::path& filepath);
		void close_project();

		void init_keybinds();
		void init_player();
		void init_options();

		void fetch_themes();
		utils::file_node fetch_scripts(const std::filesystem::path& path);

		void draw_menubar();
		void draw_project_selector();
		void draw_main_app();

		void enable_undocking(bool value);

		virtual void draw() override;
		virtual void handle_event(const SDL_Event& event) override;
	};
}
