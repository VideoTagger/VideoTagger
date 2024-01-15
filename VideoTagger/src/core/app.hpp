#pragma once
#include <string>
#include <memory>

#include <video/video.hpp>
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
		std::filesystem::path projects_list_filepath = std::filesystem::path("projects").replace_extension("json");
		std::filesystem::path app_settings_filepath = std::filesystem::path("settings").replace_extension("json");
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
		SDL_Window* main_window_;
		SDL_Renderer* renderer_;

		app_context ctx_;
		//temporary
		video vid;

	public:
		bool init(const app_config& config);
		bool run();
		void shutdown();

		bool load_settings();

		void handle_events();
		void render();

		void draw();
		void draw_project_selector();
		void draw_ui();
	};
}
