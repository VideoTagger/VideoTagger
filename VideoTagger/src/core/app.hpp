#pragma once
#include <string>
#include <memory>

#include <video/video.hpp>

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
		SDL_Window* main_window_;
		SDL_Renderer* renderer_;
		//temporary
		video vid;

	public:
		bool init(const app_config& config);
		bool run();
		void shutdown();

		void handle_events();
		void render();

		void draw();
		void draw_ui();
	};
}
