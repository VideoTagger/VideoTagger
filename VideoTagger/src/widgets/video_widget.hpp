#pragma once
#include <video/video_stream.hpp>

namespace vt::widgets
{
	extern void draw_video_widget(video_stream& video, bool& is_open, uint64_t id);
}
