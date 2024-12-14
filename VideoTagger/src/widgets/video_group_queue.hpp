#pragma once
#include <string>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	class video_group_queue
	{
	public:
		video_group_queue() = default;

	public:
		video_group_id_t current_group_id{};

	public:
		void render(bool& is_open);

		static std::string window_name();
	};
}
