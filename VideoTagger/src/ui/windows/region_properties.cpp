#include "pch.hpp"
#include "region_properties.hpp"
#include <core/app_context.hpp>
#include <ui/icons.hpp>
#include <events/player/seek_request_event.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <ui/impl/region_data_renderer.hpp>
#include <utils/vec.hpp>

namespace vt::ui::windows
{
	region_properties::region_properties() : ui::window{ "Region Properties", "region-properties", "Region Properties", ImGuiWindowFlags_NoCollapse }
	{
		set_icon(icons::shape);
	}

	void region_properties::on_render()
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

		auto video_size = utils::vec2<int>{ video_ptr->width(), video_ptr->height() };
		auto current_ts = ctx_.displayed_videos.current_timestamp_as_timestamp();

		region_renderer.render_region_attributes(get_event_source(), video_size, current_ts, selected_region);
	}
}
