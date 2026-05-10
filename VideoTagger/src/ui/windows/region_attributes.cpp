#include "pch.hpp"
#include "region_attributes.hpp"
#include <core/app_context.hpp>
#include <ui/icons.hpp>
#include <events/player/seek_request_event.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <ui/impl/region_data_renderer.hpp>
#include <utils/vec.hpp>

namespace vt::ui::windows
{
	region_attributes::region_attributes() : ui::window{ "Region Attributes", "region-attributes", "Region Attributes", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse }
	{
		set_icon(icons::shape);
	}

	void region_attributes::on_render()
	{
		if (ctx_.session.current_video_group_id() == invalid_video_group_id or !ctx_.session.is_any_region_selected())
		{
			ui::centered_text("Select a region to display its properties...", ImGui::GetContentRegionMax());
			return;
		}

		auto& selected_region = *ctx_.session.selected_region();
		auto& region_renderer = dynamic_cast<ui::impl::region_data_renderer&>(*selected_region.attribute_instance);
		auto video_ptr = ctx_.current_project->videos.get(selected_region.video_id);
		if (video_ptr == nullptr)
		{
			return;
		}

		auto video_size = utils::vec2<uint32_t>{ static_cast<uint32_t>(video_ptr->width()), static_cast<uint32_t>(video_ptr->height()) };
		auto current_ts = ctx_.displayed_videos.current_timestamp_as_timestamp();

		region_renderer.render_region_attributes(get_event_source(), video_size, current_ts, selected_region);

		/*auto selected_attr_inst = ctx_.get_selected_attribute();
		auto active_vid_size = ctx_.get_active_video_tex_size();
		auto current_ts = ctx_.displayed_videos.current_timestamp_as_timestamp();

		std::string selected_tag;
		segment_id selected_segment_id = invalid_segment_id;

		const auto& storage = ctx_.get_current_segment_storage();
		{
			bool found = false;
			for (const auto& [tag, segments] : ctx_.session.selected_segments())
			{
				if (found) break;

				const auto& timeline = storage.at(tag);
				for (auto& segment_id : segments)
				{
					const auto& segment = timeline.at(segment_id);
					if (segment.start <= current_ts and current_ts <= segment.end)
					{
						selected_tag = tag;
						selected_segment_id = segment_id;
						found = true;
						break;
					}
				}
			}
		}

		auto ev_src = get_event_source();
		if (active_vid_size.has_value() and selected_attr_inst != nullptr and selected_attr_inst->has<shape>() and selected_segment_id != invalid_segment_id)
		{
			auto& selected_seg = storage.at(selected_tag).at(selected_segment_id);
			bool is_timestamp = selected_seg.start == selected_seg.end;
			auto& shape = selected_attr_inst->get<vt::shape>();
			if (shape.has_data())
			{
				ui::card([&]()
				{
					bool modifiable = true;
					shape.draw_data(active_vid_size.value(), selected_seg.start, selected_seg.end, current_ts, is_timestamp, modifiable, ctx_.is_project_dirty, [ev_src](timestamp target_ts)
					{
						auto& player = ctx_.get_window<widgets::video_player>();
						ctx_.dispatch_event<seek_request_event>(ev_src, player, target_ts.total_milliseconds);
					});
				});
			}
			else
			{
				ui::centered_text("Select a proper shape to display its properties...", ImGui::GetContentRegionMax());
			}
		}
		else
		{
			ctx_.dispatch_event<gizmo_set_targets_event>(ev_src);
			ui::centered_text("Select a shape attribute in the inspector to display its properties...", ImGui::GetContentRegionMax());
		}*/
	}
}
