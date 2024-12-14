#pragma once
#include <functional>
#include <filesystem>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	class video_browser
	{
	public:
		std::function<void(video_id_t)> on_open_video;

		video_browser() = default;

		void render(bool& is_open);

		static std::string window_name();
	};
}
