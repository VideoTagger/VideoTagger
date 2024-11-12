#include "pch.hpp"
#include "inspector.hpp"

#include "time_input.hpp"
#include "controls.hpp"
#include <tags/tag_timeline.hpp>
#include <utils/time.hpp>
#include "icons.hpp"
#include <core/debug.hpp>

#include "video_timeline.hpp"

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

								uint64_t start_max = link_start_end ? max_timestamp : std::max<uint64_t>(0, ts_end.seconds_total.count() - 1);
								uint64_t end_min = link_start_end ? min_timestamp : ts_start.seconds_total.count() + 1;

								//ImGui::Columns(2, nullptr, false);
								bool start_activated = false;
								bool start_released = false;
								bool modified_start = timestamp_control("Start", ts_start, min_timestamp, start_max, &start_activated, &start_released);
								//ImGui::NextColumn();
								bool end_activated = false;
								bool end_released = false;
								bool modified_end = timestamp_control("End", ts_end, end_min, max_timestamp, &end_activated, &end_released);

								std::string name = icons::link + std::string("##LinkTimestamps");
								if (icon_toggle_button(name.c_str(), link_start_end))
								{
									link_start_end = !link_start_end;
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
									timestamp move_value = timestamp(std::abs(ts_start.seconds_total.count()));
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

				if (!selected_segment->tag->attributes.empty() and begin_collapsible("##Attributes", "Attributes", ImGuiTreeNodeFlags_DefaultOpen, icons::attribute))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 1, ImGuiTableFlags_RowBg))
					{
						ImGui::TableNextColumn();
						ImGui::Columns(2);
						{
							int i{};
							for (auto& [name, attr] : selected_segment->tag->attributes)
							{
								ImGui::PushID(i++);
								color_indicator(3.f, tag_attribute::type_color(attr.type_));
								ImGui::SameLine(style.ItemSpacing.x);
								//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
								//ImGui::InputText("##AttributeName", (std::string*)&name, ImGuiInputTextFlags_ReadOnly);
								ImGui::AlignTextToFramePadding();
								ImGui::TextUnformatted(name.c_str());
								ImGui::NextColumn();

								switch (attr.type_)
								{
									case tag_attribute::type::bool_:
									{
										bool v = true;
										ImGui::Checkbox("##AttributeCheckbox", &v);
									}
									break;
									case tag_attribute::type::float_:
									{
										double v = 0.5;
										ImGui::DragScalar("##AttributeFloat", ImGuiDataType_Double, &v, 1.f, nullptr, nullptr, "%g", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
									}
									break;
									case tag_attribute::type::integer:
									{
										int64_t v{};
										ImGui::DragScalar("##AttributeInt", ImGuiDataType_S64, &v, 1.f, nullptr, nullptr, nullptr, ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
									}
									break;
									case tag_attribute::type::string:
									{
										std::string v;
										ImGui::InputTextWithHint("##AttributeString", "Empty", &v);
									}
									break;
								}
								ImGui::NextColumn();

								ImGui::PopID();
							}
						}
						ImGui::Columns();
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
					end_collapsible();
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
