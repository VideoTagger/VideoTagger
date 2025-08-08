#include "pch.hpp"
#include "shape_attributes.hpp"
#include <core/app_context.hpp>
#include "controls.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>

namespace vt::widgets
{
	void shape_attributes::render(std::optional<selected_segment_data>& selected_segment, bool& is_open)
	{
		const auto& style = ImGui::GetStyle();

		if (ImGui::Begin(window_name().c_str(), &is_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ctx_.current_video_group_id() != invalid_video_group_id and ctx_.last_focused_video.has_value() and selected_segment.has_value())
			{
				auto selected_attr_inst = ctx_.get_selected_attribute();
				auto active_vid_size = ctx_.get_active_video_tex_size();

				auto& timeline = ctx_.get_current_segment_storage().at(selected_segment->tag);
				auto& selected_seg = timeline.at(selected_segment->segment_id);

				auto current_ts = ctx_.video_timeline.current_timestamp();
				bool is_on_screen = current_ts >= selected_seg.start and current_ts <= selected_seg.end;
				bool is_timestamp = selected_seg.start == selected_seg.end;
				if (active_vid_size.has_value() and selected_attr_inst != nullptr and selected_attr_inst->has<shape>() and is_on_screen)
				{
					auto& shape = selected_attr_inst->get<vt::shape>();
					if (shape.has_data())
					{
						ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
						if (ImGui::BeginTable("##Background", 1, ImGuiTableFlags_RowBg))
						{
							ImGui::TableNextColumn();
							bool modifiable = is_on_screen;
							shape.draw_data(active_vid_size.value(), ctx_.gizmo_target, selected_seg.start, selected_seg.end, current_ts, is_timestamp, modifiable, ctx_.is_project_dirty, [](timestamp target_ts)
							{
								ctx_.displayed_videos.seek(target_ts.total_milliseconds);
							});
							ImGui::EndTable();
						}
						ImGui::PopStyleColor();
					}
					else
					{
						ui::centered_text("Select a proper shape to display its properties...", ImGui::GetContentRegionMax());
					}
				}
				else
				{
					ctx_.gizmo_target = nullptr;
					ui::centered_text("Select a shape attribute in the inspector to display its properties...", ImGui::GetContentRegionMax());
				}
			}
			else
			{
				ui::centered_text("Select a shape attribute to display its properties...", ImGui::GetContentRegionMax());
			}
		}
		ImGui::End();
	}

	std::string shape_attributes::window_name()
	{
		return fmt::format("{} Shape Attributes", icons::shape);
	}
}
