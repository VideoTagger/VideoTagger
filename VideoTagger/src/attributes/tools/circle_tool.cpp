#include "pch.hpp"
#include <algorithm>
#include "circle_tool.hpp"
#include <utils/vec.hpp>
#include <core/app_context.hpp>
#include <utils/math.hpp>

namespace vt
{
	void circle_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<circle_shape>::render_overlay(video_id, pos, size, tex_size);

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
				start_mouse_pos_ = ImGui::GetMousePos();
				shape_data.emplace(to_texture_space(start_mouse_pos_, pos, size, tex_size), 0);
			}
		}
		else
		{
			if (!ImGui::IsWindowFocused()) return;

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				auto mouse_pos = ImGui::GetMousePos();

				float circle_screen_radius = std::sqrt(std::pow((mouse_pos.x - start_mouse_pos_.x), 2) + std::pow((mouse_pos.y - start_mouse_pos_.y), 2)) / 2;

				float left = pos.x;
				float right = pos.x + size.x;
				float top = pos.y;
				float bottom = pos.y + size.y;

				float max_radius_x = std::min(start_mouse_pos_.x - left, right - start_mouse_pos_.x);
				float max_radius_y = std::min(start_mouse_pos_.y - top, bottom - start_mouse_pos_.y);

				float max_radius = std::min(max_radius_x, max_radius_y);

				circle_screen_radius = std::clamp(circle_screen_radius, 0.0f, max_radius);

				shape_data->radius = math::scale_value(circle_screen_radius, 0.f, size.x, static_cast<uint32_t>(0), static_cast<uint32_t>(tex_size.x));

				const auto& tag = get_tag();
				shape_data->render(utils::vec2<uint32_t>({ (uint32_t)tex_size.x, (uint32_t)tex_size.y }), pos, pos + size, tag.fill_color(), tag.outline_color(), std::nullopt);
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				insert_region(video_id);
				reset();
			}
		}
	}
}
