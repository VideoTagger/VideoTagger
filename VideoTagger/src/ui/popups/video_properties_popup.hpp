#pragma once
#include <chrono>
#include <ui/popup.hpp>
#include <core/types.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	class video_properties_popup : public modal_popup
	{
	public:
		video_properties_popup(event_source source, std::optional<bool*> open = std::nullopt);

	private:
		event_source source_;
		video_id_t video_id_;
		video_group_id_t video_group_id_;
		std::chrono::nanoseconds offset_;

	public:
		void set_video_id(video_id_t video_id);
		void set_video_group_id(video_group_id_t video_group_id);
		void set_offset(std::chrono::nanoseconds offset);

		virtual void on_render() override;
	};
}
