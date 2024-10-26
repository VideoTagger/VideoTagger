#pragma once
#include <SDL.h>
#include <imgui.h>

namespace vt
{
	struct app_window_config
	{
		int window_width{};
		int window_height{};
		int window_pos_x{};
		int window_pos_y{};
		std::string window_name;
		bool is_tool{};
	};

	class app_window
	{
	public:
		app_window(const app_window_config& cfg);

	public:
		SDL_Window* window{};
		SDL_Renderer* renderer{};

	private:
		std::string name_;
		ImGuiContext* imgui_ctx{};

	public:
		void show(bool value = true);
		void set_darkmode(bool value = true);
		void set_current();
		void set_subtitle(const std::string& subtitle = {});

		void build_fonts(float size);
		void render();
		virtual void draw() = 0;
		void pre_handle_event();
		virtual void handle_event(const SDL_Event& event);
	};
}
