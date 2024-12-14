#pragma once
#include <video/video_stream.hpp>
#include "tag_manager.hpp"
#include "video_timeline.hpp"

namespace vt::widgets
{
	//TODO: these functions should probably be removed since other widgets aren't handled this way
	//TODO: Move into a class
	extern void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, std::optional<tag_delete_data>& tag_delete, bool& dirty_flag, bool& open);
	inline std::string tag_manager_window_name()
	{
		return fmt::format("{} Tag Manager###Tag Manager", icons::tags);
	}
}
