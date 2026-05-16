#pragma once
#include <cstddef>
#include <vector>
#include <functional>
#include <ui/window.hpp>
#include <video/video_stream.hpp>
#include <core/types.hpp>
#include <render/gl_texture.hpp>

namespace vt::ui::windows
{
	struct video_window : public window
	{
	public:
		video_window(uint64_t id);

	private:
		std::vector<std::function<void(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)>> overlays_;
		video_stream* video_;
		gl_texture* texture_;
		video_id_t video_id_;
		uint64_t id_;
		ImVec2 offset_;
		ImVec2 last_mouse_pos_;
		float scale_;
		bool is_interactive_;
		bool is_active_;

	public:
		video_window& with_overlay(const std::function<void(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)>& overlay);
		void clear_overlays();
		void render_overlays(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size);

		void set_video(video_stream& video, video_id_t video_id);
		void set_texture(gl_texture& texture);
		void set_active(bool value);

		void on_zoom(float zoom_factor, ImVec2 video_screen_pos, ImVec2 zoom_center);

		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;
	};
}
