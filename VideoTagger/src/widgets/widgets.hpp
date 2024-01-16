#pragma once
#include <video/video.hpp>
#include "tag_manager.hpp"

namespace vt::widgets
{
	extern void draw_video_widget(video& video);
	extern void draw_timeline_widget_sample(tag_storage& tags, video& video);
	extern void draw_tag_manager_widget(tag_storage& tags);
}
