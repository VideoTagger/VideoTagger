#pragma once
#include <ui/window.hpp>
#include <core/types.hpp>

namespace vt::widgets
{
	class video_group_browser : public ui::window
	{
	public:
		video_group_browser();

	public:
		video_group_id_t current_video_group{};

	public:
		void on_open_video(video_id_t video_id);

		virtual void on_render() override;
	};
}
