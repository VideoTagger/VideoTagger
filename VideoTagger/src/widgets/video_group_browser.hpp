#pragma once
#include <ui/window.hpp>
#include <core/types.hpp>
#include <ui/popups/video_properties_popup.hpp>

namespace vt::widgets
{
	class video_group_browser : public ui::window
	{
	public:
		video_group_browser();

	public:
		std::unique_ptr<ui::video_properties_popup> video_properties_popup_;
		bool open_properties_;
		video_group_id_t current_video_group{};

	public:
		void on_open_video(video_id_t video_id);

		virtual void on_render() override;

	private:
		void register_listeners();
	};
}
