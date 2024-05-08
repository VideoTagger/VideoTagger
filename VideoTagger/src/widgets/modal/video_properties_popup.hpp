#pragma once
#include <string_view>
#include <vector>
#include <video/video_pool.hpp>

namespace vt
{
	//bool video_group_offset_popup(const std::string& id, const video_pool& videos, video_group& group);
	bool video_properties_popup(const char* id, std::chrono::nanoseconds& offset);
}
