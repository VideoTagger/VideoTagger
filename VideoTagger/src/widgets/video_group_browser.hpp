#pragma once
#include <functional>
#include <filesystem>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	class video_group_browser
	{
	public:
		video_group_browser() = default;

	public:
		std::function<void(video_id_t)> on_open_video;
		video_group_id_t current_video_group{};

	public:
		void render(bool& is_open);
	};
}
