#pragma once
#include <functional>
#include <video/video_stream.hpp>

namespace vt::widgets
{
	extern void draw_video_widget(video_stream& video, const gl_texture& video_texture, bool is_video_active, bool& is_open, uint64_t id, const std::function<void(ImVec2, ImVec2, ImVec2)>& draw_overlay = nullptr);
}
