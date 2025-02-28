#include "pch.hpp"
#include "inspector.hpp"

#include "time_input.hpp"
#include "controls.hpp"
#include <tags/tag_timeline.hpp>
#include <utils/time.hpp>
#include "icons.hpp"
#include <core/debug.hpp>

#include "video_timeline.hpp"
#include <core/app_context.hpp>
#include <editor/selected_attribute_query.hpp>
#include <editor/active_video_tex_size_query.hpp>

namespace vt::widgets
{
	bool inspector(std::optional<selected_segment_data>& selected_segment, std::optional<moving_segment_data>& moving_segment, bool& link_start_end, bool& dirty_flag, bool* open, uint64_t min_timestamp, uint64_t max_timestamp)
	{
		bool result{};
		
		if (ImGui::Begin(inspector_id.c_str(), open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse) and ImGui::BeginChild("##ScrollableInspector", ImGui::GetContentRegionAvail()))
		{
			if (selected_segment.has_value())
			{
				static timestamp popup_ts_start;
				static timestamp popup_ts_end;
				auto& ts = selected_segment->segment_it;
				auto ts_start = ts->start;
				auto ts_end = ts->end;

				if (moving_segment.has_value())
				{
					ts_start = moving_segment->start;
					ts_end = moving_segment->end;
				}

				bool finished_editing = false;
				bool started_editing = false;
				bool modified_timestamp = false;
				uint8_t grab_part{};
				timestamp grab_position;
				const auto& style = ImGui::GetStyle();

				if (begin_collapsible("##Properties", "Properties", ImGuiTreeNodeFlags_DefaultOpen, icons::property))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 1, ImGuiTableFlags_RowBg))
					{
						ImGui::TableNextColumn();
						ImGui::AlignTextToFramePadding();
						ImGui::Columns(2);
						ImGui::Text("Timestamp");
						ImGui::NextColumn();
						switch (ts->type())
						{
							case tag_segment_type::timestamp:
							{
								modified_timestamp = timestamp_control("Point", ts_start, min_timestamp, max_timestamp, &started_editing, &finished_editing);
								ts_end = ts_start;
								grab_part = 0b11;
								grab_position = ts_start;
							}
							break;
							case tag_segment_type::segment:
							{
								timestamp prev_ts_start = ts_start;
								timestamp prev_ts_end = ts_end;

								uint64_t start_max = link_start_end ? max_timestamp : std::max<uint64_t>(0, ts_end.total_milliseconds.count() - 1);
								uint64_t end_min = link_start_end ? min_timestamp : ts_start.total_milliseconds.count() + 1;

								bool start_activated = false;
								bool start_released = false;
								bool end_activated = false;
								bool end_released = false;
								bool modified_start = false;
								bool modified_end = false;

								if (ImGui::BeginTable("##SegmentProperties", 2, ImGuiTableFlags_NoSavedSettings))
								{
									ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
									ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

									auto y_offset = ImVec2{ 0.f, (ImGui::GetTextLineHeight() / 2 + style.FramePadding.y) };

									ImGui::TableNextColumn();
									//ImGui::Columns(2, nullptr, false);
									modified_start = timestamp_control("Start", ts_start, min_timestamp, start_max, &start_activated, &start_released);
									ImGui::SameLine();
									auto start_pos = ImGui::GetCursorScreenPos() + y_offset;
									ImGui::NewLine();

									//ImGui::NextColumn();
									modified_end = timestamp_control("End", ts_end, end_min, max_timestamp, &end_activated, &end_released);
									ImGui::SameLine();
									auto end_pos = ImGui::GetCursorScreenPos() + y_offset;
									ImGui::NewLine();

									ImGui::TableNextColumn();
									auto icon = link_start_end ? icons::link : icons::link_off;
									std::string name = fmt::format("{}##LinkTimestamps", icon);
									
									auto icon_link_y = end_pos.y - start_pos.y + 2.125f * style.ItemSpacing.y + (ImGui::CalcTextSize(icon).y) / 2;
									ImGui::SetCursorPosY(icon_link_y);
									auto cpos = ImGui::GetCursorScreenPos();
									auto link_pos = cpos + ImGui::CalcTextSize(icon) / 2.f + style.FramePadding;

									auto drawlist = ImGui::GetWindowDrawList();
									auto line_size = 1.f;
									auto line_color_vec4 = link_start_end ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];
									line_color_vec4.w *= 0.4f;
									auto line_color = ImGui::ColorConvertFloat4ToU32(line_color_vec4);
									drawlist->AddLine(start_pos, { link_pos.x, start_pos.y }, line_color, line_size);
									drawlist->AddLine(end_pos, { link_pos.x, end_pos.y }, line_color, line_size);

									ImVec2 link_pos_offset{ 0.f, ImGui::CalcTextSize(icon).y / 4 + style.FramePadding.y };

									drawlist->AddLine({ link_pos.x, start_pos.y - line_size / 2 }, link_pos - link_pos_offset, line_color, line_size);
									drawlist->AddLine({ link_pos.x, end_pos.y + line_size / 2 }, link_pos + link_pos_offset, line_color, line_size);

									if (icon_toggle_button(name.c_str(), link_start_end))
									{
										link_start_end = !link_start_end;
									}

									ImGui::EndTable();
								}

								if (link_start_end)
								{
									if (modified_start)
									{
										ts_end += ts_start - prev_ts_start;
									}
									else if (modified_end)
									{
										ts_start += ts_end - prev_ts_end;
									}
								}

								if (ts_start > ts_end)
								{
									std::swap(ts_start, ts_end);
								}

								if (ts_start < timestamp::zero())
								{
									timestamp move_value = timestamp(std::abs(ts_start.total_milliseconds.count()));
									ts_start += move_value;
									ts_end += move_value;
								}

								modified_timestamp = modified_start or modified_end;
								started_editing = start_activated or end_activated;
								finished_editing = start_released or end_released;

								if (start_activated)
								{
									grab_part |= 0b01;
									grab_position = ts_start;
								}
								if (end_activated)
								{
									grab_part |= 0b10;
									grab_position = ts_end;
								}
							}
							break;
						}
						ImGui::Columns();
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
					end_collapsible();
				}

				if (started_editing)
				{
					moving_segment = moving_segment_data
					{
						selected_segment->tag,
						selected_segment->segment_it,
						0, // grab_part,
						grab_position,
						ts_start,
						ts_end
					};
				}

				if (modified_timestamp and moving_segment.has_value())
				{
					moving_segment->start = ts_start;
					moving_segment->end = ts_end;
				}

				if (finished_editing)
				{
					//TODO: All of this is just copied from video timeline. Probably should do something about this
					auto& timeline = selected_segment->segments;
					auto overlapping = timeline->find_range(ts_start, ts_end);

					bool insert_now = true;
					for (auto it = overlapping.begin(); it != overlapping.end(); ++it)
					{
						if (it != selected_segment->segment_it)
						{
							insert_now = false;
						}
					}

					if (insert_now)
					{
						if (ts->start != ts_start or ts->end != ts_end)
						{
							ts = timeline->replace(ts, ts_start, ts_end).first;
							dirty_flag = true;
						}

						moving_segment.reset();
					}
					else
					{
						ImGui::OpenPopup("##MergeSegments");
						popup_ts_start = ts_start;
						popup_ts_end = ts_end;
					}
				}

				bool pressed_yes{};
				if (merge_segments_popup("##MergeSegments", pressed_yes, true))
				{
					if (pressed_yes)
					{
						auto& timeline = selected_segment->segments;
						ts = timeline->replace(ts, popup_ts_start, popup_ts_end).first;

						dirty_flag = true;
					}
					moving_segment.reset();
				}

				if (ctx_.current_video_group_id() != invalid_video_group_id and ctx_.last_focused_video.has_value())
				{
					ImGui::BeginDisabled(selected_segment->tag->attributes.empty());
					selected_segment->tag->draw_attribute_instances(*selected_segment->segment_it, ctx_.last_focused_video.value(), ctx_.is_project_dirty);
					ImGui::EndDisabled();
				}
			}
			else
			{
				centered_text("Select a segment to display its properties...", ImGui::GetContentRegionMax());
			}
			ImGui::EndChild();
		}
		ImGui::End();

		return result;
	}
}
