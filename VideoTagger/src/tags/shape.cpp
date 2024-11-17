#include "pch.hpp"
#include "shape.hpp"

#include <limits>
#include <imgui.h>
#include <widgets/controls.hpp>
#include <utils/vec.hpp>

namespace vt
{
	static bool positon_control(utils::vec2<uint32_t>& pos, const utils::vec2<uint32_t>& max_size)
	{
		bool result{};
		const auto& style = ImGui::GetStyle();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ style.ItemSpacing.x / 2, style.CellPadding.y });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 xysize = { line_height + 3.0f, line_height };

		if (ImGui::BeginTable("##PositionControl", 2))
		{
			ImGui::TableNextColumn();
			if (ImGui::Button("X", xysize))
			{
				pos[0] = 0;
				result = true;
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			float x = (float)pos[0];
			if (ImGui::DragScalar("##x", ImGuiDataType_U32, &pos[0], 1.f, 0, &max_size[0], "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				pos[0] = std::clamp(pos[0], 0u, max_size[0]);
				result = true;
			}
			
			ImGui::TableNextColumn();
			if (ImGui::Button("Y", xysize))
			{
				pos[1] = 0;
				result = true;
			}
			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::DragScalar("##y", ImGuiDataType_U32, &pos[1], 1.f, 0, &max_size[1], "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				pos[1] = std::clamp(pos[1], 0u, max_size[1]);
				result = true;
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);
		return result;
	}

	void shape::draw_data(const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, bool& dirty_flag)
	{
		const auto& style = ImGui::GetStyle();
		//TODO: Improve this UI

		switch (type_)
		{
			case shape::type::none: return;
			case shape::type::circle:
			{
				auto& v = get<circle>();
				gizmo_target = &v.pos;

				ImGui::Columns(2);
				ImGui::TextUnformatted("Position");
				ImGui::NextColumn();

				positon_control(v.pos, max_size);

				ImGui::NextColumn();
				ImGui::TextUnformatted("Radius");
				ImGui::NextColumn();
				ImGui::DragFloat("##Radius", &v.radius, 1.f, 1.f, 0.f, "%g", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Columns();
			}
			break;
			case shape::type::rectangle:
			{
				auto& v = get<rectangle>();
				ImGui::Columns(2);


				ImGui::Columns();
			}
			break;
			case shape::type::polygon:
			{
				auto& v = get<polygon>();
			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
	}
}
