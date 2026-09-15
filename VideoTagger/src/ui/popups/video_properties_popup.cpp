#include "video_properties_popup.hpp"
#include <utils/timestamp.hpp>
#include <widgets/time_input.hpp>
#include <ui/widgets/button_bar.hpp>
#include <core/app_context.hpp>

#include <events/video_group/video_change_offset_request_event.hpp>

namespace vt::ui
{
	video_properties_popup::video_properties_popup(event_source source, std::optional<bool*> open) : modal_popup{ "video-properties", open },
		source_{ source }, video_group_id_{ invalid_video_group_id }, video_id_{}, offset_ {} {}

	void video_properties_popup::set_video_id(video_id_t video_id)
	{
		video_id_ = video_id;
	}

	void video_properties_popup::set_video_group_id(video_group_id_t video_group_id)
	{
		video_group_id_ = video_group_id;
	}

	void video_properties_popup::set_offset(std::chrono::nanoseconds offset)
	{
		offset_ = offset;
	}

	void video_properties_popup::on_render()
	{
		timestamp ts(offset_);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Offset");
		
		widgets::time_input("##VideoOffsetInput", &ts);
		offset_ = ts.total_nanoseconds;

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("generic.confirm") },
			{ 1, ctx_.lang->get("generic.cancel") },
		};

		close_on_escape();
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
				case 0:
				{
					ctx_.dispatch_event<video_change_offset_request_event>(source_, video_group_id_, video_id_, offset_);
					close();
				}
				break;
				default: close(); break;
			}
		});
	}
}
