#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <memory>
#include <type_traits>
#include <ui/popup.hpp>
#include <utils/vec.hpp>

namespace vt
{
	struct system_window_config
	{
		int window_width{};
		int window_height{};
		int window_pos_x{};
		int window_pos_y{};
		std::string window_name;
		bool is_tool{};
	};

	class system_window
	{
	public:
		system_window(const system_window_config& cfg);
		~system_window();

	public:
		SDL_Window* window{};
		SDL_GLContext gl_ctx{};

	private:
		std::string name_;

	public:
		void show(bool value = true);
		void set_darkmode(bool value = true);
		void set_current();
		void set_subtitle(const std::string& subtitle = {});

		void build_fonts(float size);
		void render();

		void set_position(const utils::vec2<int>& position);
		void set_size(const utils::vec2<int>& size);
		void center();
		void maximize();
		void minimize();
		void restore();

		utils::vec2<int> position() const;
		utils::vec2<int> size() const;

		bool operator==(const system_window& other) const;

		virtual void pre_render() {};
		virtual void on_render() = 0;
		virtual void handle_event(const SDL_Event& event);
	};
}
