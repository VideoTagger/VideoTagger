#include "pch.hpp"
#include "shape.hpp"

#include <limits>
#include <imgui.h>
#include <functional>
#include <widgets/controls.hpp>
#include <utils/vec.hpp>
#include <utils/intersection.hpp>

namespace vt
{
	void draw_vertex_list(std::vector<utils::vec2<uint32_t>>& vertices, const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, bool modifiable, const std::function<void(utils::vec2<uint32_t>&)>& on_select)
	{
		const auto& style = ImGui::GetStyle();

		ImVec2 separator_pos;
		if (ImGui::BeginTable("##VertexList", 2, ImGuiTableFlags_BordersOuter))
		{
			ImGui::TableNextColumn();
			if (modifiable and widgets::icon_button(icons::add))
			{
				vertices.push_back({});
				gizmo_target = nullptr;
			}
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Vertices");
			separator_pos = ImGui::GetCursorPos() - ImVec2{ 0.f, style.ItemSpacing.y / 2 + 0.5f };
			ImGui::TableNextColumn();

			for (size_t i = 0; i < vertices.size(); ++i)
			{
				ImGui::PushID(&vertices[i]);
				bool selected = (gizmo_target == &vertices[i]);
				ImGui::TableNextColumn();
				auto cpos = ImGui::GetCursorPos();
				if (ImGui::Selectable("##VertexSelectable", selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns, { 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y }))
				{
					on_select(vertices[i]);
				}
				ImGui::SetCursorPos(cpos);

				ImGui::AlignTextToFramePadding();
				ImGui::Indent();
				ImGui::TextUnformatted("Vertex");
				ImGui::Unindent();
				ImGui::SameLine();
				ImGui::BeginDisabled();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("[%zu]", i + 1);
				ImGui::EndDisabled();
				
				ImGui::TableNextColumn();
				widgets::positon_control(vertices[i], max_size);
				ImGui::PopID();
			}
			ImGui::EndTable();
			if (!vertices.empty())
			{
				ImGui::SetCursorPos(separator_pos);
				ImGui::Separator();
			}
		}
	}

	void shape::draw(const std::function<ImVec2(const ImVec2&)>& to_local_pos, const ImVec2& tex_size, const ImVec2& viewport_size, uint32_t outline_color, uint32_t fill_color, bool show_points, bool& is_mouse_over) const
	{
		float point_size = 5.f;

		static auto to_imvec2 = [](const utils::vec2<uint32_t>& v) -> ImVec2
		{
			return ImVec2{ (float)v[0], (float)v[1] };
		};

		auto draw_list = ImGui::GetWindowDrawList();
		auto mouse_pos = ImGui::GetMousePos();

		switch (type_)
		{
			case shape::type::none: return;
			case shape::type::circle:
			{
				auto& v = get<circle>();
				float viewport_diagonal = std::sqrt(viewport_size.x * viewport_size.x + viewport_size.y * viewport_size.y);
				float tex_diagonal = std::sqrt(tex_size.x * tex_size.x + tex_size.y * tex_size.y);
				float scaled_radius = (float)v.radius * viewport_diagonal / tex_diagonal;

				auto local_pos = to_local_pos(to_imvec2(v.pos));

				draw_list->AddCircleFilled(local_pos, scaled_radius, fill_color);
				draw_list->AddCircle(local_pos, scaled_radius, outline_color);

				if (utils::intersection::is_in_circle(mouse_pos, local_pos, scaled_radius))
				{
					is_mouse_over = true;
				}

				if (show_points)
				{
					draw_list->AddCircleFilled(local_pos, point_size, 0xFFFFFFFF);
				}
			}
			break;
			case shape::type::rectangle:
			{
				auto& v = get<rectangle>();
				auto min = to_local_pos(to_imvec2(v.vertices.at(0)));
				auto max = to_local_pos(to_imvec2(v.vertices.at(1)));

				draw_list->AddRectFilled(min, max, fill_color);
				draw_list->AddRect(min, max, outline_color);

				if (utils::intersection::is_in_rect(mouse_pos, ImRect{ min, max }))
				{
					is_mouse_over = true;
				}

				if (show_points)
				{
					auto size = max - min;
					for (const auto& point : { min, min + ImVec2{ size.x, 0.f} , min + ImVec2{ 0.f, size.y }, max })
					{
						draw_list->AddCircleFilled(point, point_size, 0xFFFFFFFF);
					}
				}
			}
			break;
			case shape::type::polygon:
			{
				auto& v = get<polygon>();
				std::vector<ImVec2> vertices(v.vertices.size());
				auto vert_int_size = static_cast<int>(vertices.size());
				for (size_t i = 0; i < vertices.size(); ++i)
				{
					vertices[i] = to_local_pos(to_imvec2(v.vertices.at(i)));
				}

				if (utils::intersection::is_in_polygon(mouse_pos, vertices))
				{
					is_mouse_over = true;
				}

				if (utils::intersection::is_convex_polygon(vertices))
				{
					draw_list->AddConvexPolyFilled(vertices.data(), vert_int_size, fill_color);
				}
				else
				{
					draw_list->AddConcavePolyFilled(vertices.data(), vert_int_size, fill_color);
				}
				draw_list->AddPolyline(vertices.data(), vert_int_size, outline_color, ImDrawFlags_Closed, 1.f);

				if (show_points)
				{
					for (const auto& vertex : vertices)
					{
						draw_list->AddCircleFilled(vertex, point_size, 0xFFFFFFFF);
					}
				}
			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
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

				widgets::positon_control(v.pos, max_size);

				ImGui::NextColumn();
				ImGui::TextUnformatted("Radius");
				ImGui::NextColumn();
				auto max = std::min(max_size[0], max_size[1]) / 2;
				auto min = 1u;
				if (ImGui::DragScalar("##y", ImGuiDataType_U32, &v.radius, 1.f, &min, &max, "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					v.radius = std::clamp(v.radius, min, max);
				}
				ImGui::Columns();
			}
			break;
			case shape::type::rectangle:
			{
				auto& v = get<rectangle>();
				draw_vertex_list(v.vertices, max_size, gizmo_target, false, [&gizmo_target](utils::vec2<uint32_t>& vertex)
				{
					gizmo_target = &vertex;
				});
			}
			break;
			case shape::type::polygon:
			{
				auto& v = get<polygon>();
				draw_vertex_list(v.vertices, max_size, gizmo_target, true, [&gizmo_target](utils::vec2<uint32_t>& vertex)
				{
					gizmo_target = &vertex;
				});
			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
	}
}
