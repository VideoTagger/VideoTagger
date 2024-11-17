#include "pch.hpp"
#include "shape.hpp"

#include <imgui.h>

namespace vt
{
	void shape::draw_data(const ImVec2& max_size, bool& dirty_flag)
	{
		//TODO: Improve this UI

		switch (type_)
		{
			case shape::type::none: return;
			case shape::type::circle:
			{
				auto& v = get<circle>();
				ImGui::Columns(3, nullptr, false);
				ImGui::DragFloat("X", &v.pos.x, 1.f, 0.f, max_size.x, "%g", ImGuiSliderFlags_AlwaysClamp);
				ImGui::NextColumn();
				ImGui::DragFloat("Y", &v.pos.y, 1.f, 0.f, max_size.y, "%g", ImGuiSliderFlags_AlwaysClamp);
				ImGui::NextColumn();
				ImGui::DragFloat("Radius", &v.radius, 1.f, 1.f, 0.f, "%g", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Columns();
			}
			break;
			case shape::type::rectangle:
			{

			}
			break;
			case shape::type::polygon:
			{

			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
	}
}
