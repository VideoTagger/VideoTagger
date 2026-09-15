#pragma once
#include <string>
#include <ui/window.hpp>
#include <core/types.hpp>

namespace vt::widgets
{
	class video_group_queue : public ui::window
	{
	public:
		video_group_queue();

	private:
		video_group_id_t current_group_id_{};

	public:
		void set_current_group_id(video_group_id_t id);

		video_group_id_t current_group_id() const;

		virtual void pre_render() override;
		virtual void on_render() override;

		virtual void pre_style() override;
		virtual void post_style() override;
	};
}
