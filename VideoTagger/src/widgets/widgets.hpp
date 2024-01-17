#pragma once
#include <video/video.hpp>
#include "tag_manager.hpp"

namespace vt::widgets
{
	extern void draw_timeline_widget_sample(video& video, tag_storage& tags, uint32_t id);
	extern void draw_tag_manager_widget(tag_storage& tags);
}
