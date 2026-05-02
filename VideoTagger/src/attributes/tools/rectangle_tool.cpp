#include "pch.hpp"
#include <algorithm>
#include "rectangle_tool.hpp"
#include <utils/vec.hpp>

namespace vt
{
	void rectangle_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<rectangle_shape>::render_overlay(video_id, pos, size, tex_size);

		auto& shape_data = data();

		//TODO: Move this somewhere outside
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size, bool clamp = true) -> utils::vec2<uint32_t>
		{
			const float scale_x = (size.x > 0.f) ? (tex_size.x / size.x) : 0.f;
			const float scale_y = (size.y > 0.f) ? (tex_size.y / size.y) : 0.f;
			
			if (clamp)
			{
				const float tex_x = (screen_pos.x - pos.x) * scale_x;
				const float tex_y = (screen_pos.y - pos.y) * scale_y;

				return
				{
					(uint32_t)std::clamp(tex_x, 0.f, std::max(0.f, tex_size.x)),
					(uint32_t)std::clamp(tex_y, 0.f, std::max(0.f, tex_size.y))
				};
			}
			else
			{
				return
				{
					(uint32_t)((screen_pos.x - pos.x) * scale_x),
					(uint32_t)((screen_pos.y - pos.y) * scale_y)
				};
			}
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
				ImRect draw_rect{ pos, pos + size };
				shape_data->render(utils::vec2<uint32_t>({ (uint32_t)tex_size.x, (uint32_t)tex_size.y }), draw_rect, tag.outline_color(), tag.fill_color(), std::nullopt);
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				//TODO: Actually insert the region into the attribute instance instead of just resetting the shape data
				reset();
			}
		}
	}
}
