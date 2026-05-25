#include "region_list.hpp"

#include "pch.hpp"
#include <core/app_context.hpp>
#include <ui/icons.hpp>
#include <events/player/seek_request_event.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <ui/impl/region_data_renderer.hpp>
#include <utils/vec.hpp>

#include <events/attributes/region_select_request_event.hpp>

namespace vt::ui::windows
{
	static void show_player_video_ids(bool value)
	{
		auto& player = ctx_.get_window<widgets::video_player>();
		player.set_show_video_ids(value);
	}

	region_list::region_list() : ui::window{ "Segment Regions", "segment-regions", "Segment Regions", ImGuiWindowFlags_NoCollapse }
	{
		set_icon(icons::object);
	}

	void region_list::on_render()
	{
		if (ctx_.session.current_video_group_id() == invalid_video_group_id or !ctx_.session.is_one_segment_selected())
		{
			ui::centered_text("Select a single segment to display its regions...", ImGui::GetContentRegionMax());
			return;
		}

		auto selected_segment_opt = ctx_.session.any_selected_segment();
		if (!selected_segment_opt.has_value()) return;

		const auto& [tag_name, segment_id] = *selected_segment_opt;
		auto& segment_attribute_instances = ctx_.get_current_segment_storage().at(tag_name).segment_attribute_instances(segment_id);

		auto collapsible_flags = ImGuiTreeNodeFlags_DefaultOpen;

		show_player_video_ids(is_focused() and !ctx_.displayed_videos.empty());
		const auto& theme = ctx_.current_theme;
		const auto& style = ImGui::GetStyle();

		size_t video_index = 1;

		for (auto video_data_it = ctx_.displayed_videos.begin(); video_data_it != ctx_.displayed_videos.end(); ++video_data_it, ++video_index)
		{
			auto& video_data = *video_data_it;
			auto vid_id = video_data.id;
			auto video_name = ctx_.current_project->videos.get(vid_id)->title();

			std::vector<vt::impl::shape_attribute_instance*> instances_;

			auto video_instances_it = segment_attribute_instances.find(vid_id);
			if (video_instances_it == segment_attribute_instances.end()) continue;

			for (auto& attr_instance : video_instances_it->second)
			{
				if (dynamic_cast<impl::region_data_renderer*>(attr_instance.get()) == nullptr) continue;
				auto* shape_attr_instance = dynamic_cast<vt::impl::shape_attribute_instance*>(attr_instance.get());

				instances_.push_back(shape_attr_instance);
			}

			if (instances_.empty()) continue;

			auto vid_id_attrs_id = fmt::format("##Attributes-{}", vid_id);
			if (!widgets::begin_collapsible(vid_id_attrs_id, video_name, collapsible_flags, icons::video, std::nullopt, nullptr, video_index)) continue;

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ style.CellPadding.x + style.ItemSpacing.x, style.CellPadding.y });
			ImGui::PushStyleColor(ImGuiCol_TableRowBg, theme.get_float4(theme_color::background_tertiary));
			ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, theme.get_float4(theme_color::background_tertiary));

			for (auto& instance : instances_)
			{
				auto* region_data_renderer = dynamic_cast<impl::region_data_renderer*>(instance);
				const auto& attr_type_name = instance->attribute_type_name();
				const auto& attr_name = instance->attribute_name();
				auto attr_color = ctx_.attr_registry.get_attr_spec(attr_type_name)->color;

				std::optional<region_id_t> selected_region;
				if (region_data_renderer->render_region_list(get_event_source(), attr_name, attr_color, instance, selected_region))
				{
					ctx_.dispatch_event<region_select_request_event>(get_event_source(), tag_name, segment_id, vid_id, instance, *selected_region);
				}
			}

			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar();
			widgets::end_collapsible();
		}
	}
}
