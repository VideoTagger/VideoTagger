#pragma once
#include <ui/window.hpp>
#include <core/types.hpp>

namespace vt::widgets
{
	struct video_browser : public ui::window
	{
	public:
		video_browser();

	public:
		void on_open_video(video_id_t video_id);

		virtual void on_render() override;
	};
}
