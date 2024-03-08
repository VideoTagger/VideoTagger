#include "inspector.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "time_input.hpp"
#include "buttons.hpp"
#include <tags/tag_timeline.hpp>
#include <utils/time.hpp>
#include "icons.hpp"
#include <core/debug.hpp>

#include "video_timeline.hpp"

namespace vt::widgets
{
	static bool show_timestamp_control(const std::string& name, timestamp& timestamp, uint64_t min_timestamp, uint64_t max_timestamp, bool& was_activated, bool& was_released, bool fill_area = true)
	{
		bool result = false;
		auto cstr = name.c_str();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, ImGui::GetStyle().ItemSpacing.y });
		if (ImGui::Button(cstr))
		{
			timestamp = vt::timestamp(min_timestamp);
			result = true;
		}
		ImGui::SameLine();
		if (fill_area) ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		auto time_input_id = "##TimestampCtrlInput" + name;
		result |= widgets::time_input(time_input_id.c_str(), &timestamp, 1.0f, min_timestamp, max_timestamp, utils::time::default_time_format, ImGuiSliderFlags_AlwaysClamp);
		was_activated = ImGui::IsItemActivated();
		was_released = ImGui::IsItemDeactivated();
		if (fill_area) ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		auto ctx_name = ("##TimestampCtrlCtx" + name);
		if (ImGui::BeginPopupContextItem(ctx_name.c_str()))
		{
			if (ImGui::MenuItem("Set Min"))
			{
				timestamp = vt::timestamp(min_timestamp);
				result = true;
			}
			if (ImGui::MenuItem("Set Max"))
			{
				timestamp = vt::timestamp(max_timestamp);
				result = true;
			}
			ImGui::EndPopup();
		}
		return result;
	}

	bool inspector(std::optional<selected_timestamp_data>& selected_timestamp, std::optional<moving_timestamp_data>& moving_timestamp, bool& link_start_end, bool& dirty_flag, bool* open, uint64_t min_timestamp, uint64_t max_timestamp)
	{
		bool result{};
		if (ImGui::Begin("Inspector", open, ImGuiWindowFlags_NoCollapse))
		{
			if (selected_timestamp.has_value())
			{
				static timestamp popup_ts_start;
				static timestamp popup_ts_end;
				auto& ts = selected_timestamp->timestamp;
				auto ts_start = ts->start;
				auto ts_end = ts->end;

				if (moving_timestamp.has_value())
				{
					ts_start = moving_timestamp->left_position;
					ts_end = moving_timestamp->right_position;
				}

				bool finished_editing = false;
				bool started_editing = false;
				bool modified_timestamp = false;
				uint8_t grab_part{};
				timestamp grab_position;

				if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Columns(2);
					ImGui::Indent();
					ImGui::Text("Timestamp");
					ImGui::Unindent();
					ImGui::NextColumn();
					switch (ts->type())
					{
						case tag_timestamp_type::point:
						{
							modified_timestamp = show_timestamp_control("Point", ts_start, min_timestamp, max_timestamp, started_editing, finished_editing);
							ts_end = ts_start;
							grab_part = 0b11;
							grab_position = ts_start;
						}
						break;
						case tag_timestamp_type::segment:
						{
							timestamp prev_ts_start = ts_start;
							timestamp prev_ts_end = ts_end;

							//ImGui::Columns(2, nullptr, false);
							bool start_activated = false;
							bool start_released = false;
							bool modified_start = show_timestamp_control("Start", ts_start, min_timestamp, std::max<uint64_t>(0, ts_end.seconds_total.count() - 1), start_activated, start_released);
							//ImGui::NextColumn();
							bool end_activated = false;
							bool end_released = false;
							bool modified_end = show_timestamp_control("End", ts_end, ts_start.seconds_total.count() + 1, max_timestamp, end_activated, end_released);
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
							
							if (ts_start < timestamp(0))
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
				}

				if (started_editing)
				{
					moving_timestamp = moving_timestamp_data
					{
						selected_timestamp->tag,
						selected_timestamp->timestamp,
						0, // grab_part,
						grab_position,
						ts_start,
						ts_end
					};
				}

				if (modified_timestamp and moving_timestamp.has_value())
				{
					moving_timestamp->left_position = ts_start;
					moving_timestamp->right_position = ts_end;
				}

				if (finished_editing)
				{
					//TODO: All of this is just copied from video timeline. Probably should do something about this
					auto& timeline = selected_timestamp->timestamp_timeline;
					auto overlapping = timeline->find_range(ts_start, ts_end);

					bool insert_now = true;
					for (auto it = overlapping.begin(); it != overlapping.end(); ++it)
					{
						if (it != selected_timestamp->timestamp)
						{
							insert_now = false;
						}
					}

					if (insert_now)
					{
						ts = timeline->replace(ts, ts_start, ts_end).first;
						moving_timestamp.reset();
						dirty_flag = true;
					}
					else
					{
						ImGui::OpenPopup("Merge Overlapping");
						popup_ts_start = ts_start;
						popup_ts_end = ts_end;
					}
				}

				bool pressed_yes{};
				if (merge_timestamps_popup(pressed_yes))
				{
					if (pressed_yes)
					{
						auto& timeline = selected_timestamp->timestamp_timeline;
						ts = timeline->replace(ts, popup_ts_start, popup_ts_end).first;

						dirty_flag = true;
					}
					moving_timestamp.reset();
				}
			}
			else
			{
				auto avail_area = ImGui::GetContentRegionMax();
				constexpr const char* text = "Select a segment to display its properties...";
				auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
				ImGui::SetCursorPos(avail_area / 2 - half_text_size);
				ImGui::BeginDisabled();
				ImGui::TextWrapped(text);
				ImGui::EndDisabled();
			}
		}
		ImGui::End();

		return result;
	}
}
