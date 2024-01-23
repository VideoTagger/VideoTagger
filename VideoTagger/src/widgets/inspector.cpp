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
		result |= widgets::time_input(("##TimestampCtrlInput" + name).c_str(), &timestamp, 1.0f, min_timestamp, max_timestamp);
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
			if (true || selected_timestamp_data.has_value())
			{
				static timestamp time{};
				//auto ts = selected_timestamp_data->timestamp;

				if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Columns(2);
					ImGui::Indent();
					ImGui::Text("Timestamp");
					ImGui::Unindent();
					ImGui::NextColumn();
					switch (tag_timestamp_type::segment) //ts->type()
					{
						case tag_timestamp_type::point:
						{
							show_timestamp_control("Point", time, min_timestamp, max_timestamp);
						}
						break;
						case tag_timestamp_type::segment:
						{
							//ImGui::Columns(2, nullptr, false);
							show_timestamp_control("Start", time, min_timestamp, max_timestamp);
							//ImGui::NextColumn();
							show_timestamp_control("End", time, min_timestamp, max_timestamp);
							//ImGui::Columns();
							//show_timestamp_control("Duration", time, min_timestamp, max_timestamp);
						}
						break;
					}
					ImGui::Columns();
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
