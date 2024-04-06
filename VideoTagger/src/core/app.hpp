#pragma once
#include <string>
#include <memory>

#include <video/video_stream.hpp>
#include "app_context.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace vt
{
	struct app_config
	{
		int window_width{};
		int window_height{};
		int window_pos_x{};
		int window_pos_y{};
		std::string window_name;
	};

	enum class app_state
	{
		uninitialized,
		initialized,
		running,
		shutdown
	};

	class app
	{
	public:
		app();

	private:
		app_state state_;

	public:
		bool init(const app_config& config);
		bool run();
		void shutdown();
		void on_close_project(bool should_shutdown);
		void on_save();
		void on_save_as();
		void on_import_videos();
		void on_delete();

		bool load_settings();
		void save_settings();
		void save_project();
		void save_project_as(const std::filesystem::path& filepath);
		void close_project();

		void fetch_themes();
		void build_fonts(float size);
		void init_keybinds();
		void init_player();
		void init_options();
		void handle_events();
		void render();

		void draw();
		void draw_menubar();
		void draw_project_selector();
		void draw_main_app();

		void set_subtitle(const std::string& title = std::string{});
	};
}
