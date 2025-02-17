#include "pch.hpp"
#include "timeline.hpp"
#include "icons.hpp"

namespace vt::widgets
{
	void timeline::draw_marker()
	{
		auto draw_list = ImGui::GetWindowDrawList();
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		auto win_pos = ImGui::GetWindowPos();
		auto scroll_y = ImGui::GetScrollY();
		vMin.x += win_pos.x;
		vMin.y += win_pos.y;
		vMax.x += win_pos.x;
		vMax.y += win_pos.y;

		auto x = win_pos.x + ImGui::GetCursorPosX();

		//draw_list->AddRect(vMin, vMax, marker_color);

		ImVec2 top{ x, vMin.y + scroll_y };
		ImVec2 bottom{ x, vMax.y + scroll_y };

		static constexpr float marker_width = 4.f;
		static constexpr float triangle_span = marker_width * 2;
		auto item_height = ImGui::GetTextLineHeightWithSpacing();
		auto marker_pos = x;

		bool enabled = true;
		ImU32 marker_color = enabled ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
		draw_list->AddLine(top, bottom, marker_color, marker_width);
		draw_list->AddTriangleFilled
		(
			ImVec2{ marker_pos - triangle_span, top.y },
			ImVec2{ marker_pos, top.y + item_height * 0.5f },
			ImVec2{ marker_pos + triangle_span, top.y }, marker_color
		);
	}

	void timeline::render(bool& is_open)
	{
		auto& style = ImGui::GetStyle();

		auto win_name = window_name();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ style.WindowPadding.x / 2.f, 0.f });
		auto win_open = ImGui::Begin(win_name.c_str(), &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();

		if (win_open)
		{
			ImGui::Text("%s", "Test text");
			ImGui::SameLine();
			ImGui::SliderFloat("Zoom", &zoom_, 0.f, 1.f);
			ImGui::Separator();
			if (ImGui::BeginTable("##TimelineSplitter", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupColumn("Tag Name", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);

				for (size_t i = 0; i < 50; ++i)
				{
					ImGui::TableNextRow();
					//Left panel
					ImGui::TableNextColumn();
					ImGui::TextUnformatted("Tag Name");

					//Right panel
					ImGui::TableNextColumn();
										
					if (i & 1)
					{
						ImGui::Button("Segment\nSegment details", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
					}
					else
					{
						ImGui::Button("Segment");
					}
				}

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
				draw_marker();
				ImGui::EndTable();
			}
		}
		ImGui::End();

	}

	std::string timeline::window_name()
	{
		return fmt::format("{} Timeline###Timelinev2", icons::timeline);
	}
}
