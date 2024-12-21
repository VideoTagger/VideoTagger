#pragma once
#include <functional>
#include <filesystem>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	struct video_browser
	{
		video_browser() = default;

		std::function<void(video_id_t)> on_open_video;

		void render(bool& is_open);

		static std::string window_name();
	};
}
