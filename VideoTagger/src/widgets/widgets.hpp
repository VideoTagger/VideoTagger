#pragma once
#include <video/video_stream.hpp>
#include "tag_manager.hpp"
#include "video_timeline.hpp"

namespace vt::widgets
{
	//TODO: these functions should probably be removed since other widgets aren't handled this way
	extern void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, bool& dirty_flag, bool& open);
}
