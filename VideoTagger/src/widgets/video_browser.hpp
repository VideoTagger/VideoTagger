#pragma once
#include <functional>
#include <filesystem>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	class video_browser
	{
	public:
		video_browser() = default;

	public:
		std::function<void(video_id_t)> on_open_video;

	public:
		void render(bool& is_open);

		static std::string window_name();
	};
}
