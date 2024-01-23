#include "inspector.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "time_input.hpp"

namespace vt::widgets
{
	static bool show_timestamp_control(const std::string& name, timestamp& timestamp, uint64_t min_timestamp, uint64_t max_timestamp, bool fill_area = true)
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
		result |= widgets::time_input(("##TimestampCtrlInput" + name).c_str(), &timestamp, 1.0f, min_timestamp, max_timestamp, widgets::time_input_default_format, ImGuiSliderFlags_AlwaysClamp);
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

	bool inspector(std::optional<selected_timestamp_data>& selected_timestamp_data, bool* open, uint64_t min_timestamp, uint64_t max_timestamp)
	{
		bool result{};
		if (ImGui::Begin("Inspector", open, ImGuiWindowFlags_NoCollapse))
		{
			if (selected_timestamp_data.has_value())
			{
				static timestamp time{};
				auto& ts = selected_timestamp_data->timestamp;
				auto ts_start = ts->start;
				auto ts_end = ts->end;

				bool modified_timestamp = false;

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
							modified_timestamp = show_timestamp_control("Point", ts_start, min_timestamp, max_timestamp);
						}
						break;
						case tag_timestamp_type::segment:
						{
							//ImGui::Columns(2, nullptr, false);
							modified_timestamp |= show_timestamp_control("Start", ts_start, min_timestamp, ts_end.seconds_total.count());
							//ImGui::NextColumn();
							modified_timestamp |= show_timestamp_control("End", ts_end, ts_start.seconds_total.count() + 1, max_timestamp);
							//ImGui::Columns();
							//show_timestamp_control("Duration", time, min_timestamp, max_timestamp);
						}
						break;
					}
					ImGui::Columns();
				}

				if (modified_timestamp)
				{
					auto& timeline = selected_timestamp_data->timestamp_timeline;
					timeline->erase(ts);
					ts = timeline->insert(ts_start, ts_end).first;
				}
			}
			else
			{
				auto avail_area = ImGui::GetContentRegionMax();
				constexpr const char* text = "Select a segment to display its properties...";
				auto half_text_size = ImGui::CalcTextSize(text) / 2;
				ImGui::SetCursorPos(avail_area / 2 - half_text_size);
				ImGui::TextDisabled(text);
			}
		}
		ImGui::End();

		return result;
	}
}
