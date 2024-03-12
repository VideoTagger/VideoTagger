#pragma once
#include <video/video.hpp>
#include "tag_manager.hpp"
#include "video_timeline.hpp"

namespace vt::widgets
{
	extern void draw_timeline_widget_sample(timeline_state& state, video& video, tag_storage& tags, std::optional<selected_timestamp_data>& selected_timestamp, std::optional<moving_timestamp_data>& moving_timestamp, bool& dirty_flag, uint64_t id);
	extern void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, bool& dirty_flag);
}
