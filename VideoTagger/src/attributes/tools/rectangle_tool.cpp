#include "pch.hpp"
#include <algorithm>
#include "rectangle_tool.hpp"
#include <utils/vec.hpp>
#include <core/app_context.hpp>
#include <utils/math.hpp>

namespace vt
{
	void rectangle_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<rectangle_shape>::render_overlay(video_id, pos, size, tex_size);

		auto& shape_data = data();

		//TODO: Move this somewhere outside
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<uint32_t>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<uint32_t>{}, utils::vec2<uint32_t>{ static_cast<uint32_t>(tex_size.x), static_cast<uint32_t>(tex_size.y) });
		};

		if (!shape_data.has_value())
		{
			if (!ImGui::IsWindowHovered()) return;

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				auto click_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
				shape_data.emplace(click_pos, click_pos);
			}
		}
		else
		{
			if (!ImGui::IsWindowFocused()) return;

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				auto mouse_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
				shape_data->end = mouse_pos;

				const auto& tag = get_tag();
				shape_data->render(utils::vec2<uint32_t>({ (uint32_t)tex_size.x, (uint32_t)tex_size.y }), pos, pos + size, tag.fill_color(), tag.outline_color(), std::nullopt);
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				// Explicit type because without it minmax returns std::pair<const uint32_t&, const uint32_t&> which later breaks the assignment
				std::pair<uint32_t, uint32_t> minmax_x = std::minmax(shape_data->start[0], shape_data->end[0]);
				std::pair<uint32_t, uint32_t> minmax_y = std::minmax(shape_data->start[1], shape_data->end[1]);

				shape_data->start = { minmax_x.first, minmax_y.first };
				shape_data->end = { minmax_x.second, minmax_y.second };
				insert_region(video_id);
				reset();
			}
		}
	}
}
