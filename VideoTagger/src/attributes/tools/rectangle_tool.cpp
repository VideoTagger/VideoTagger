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

		auto shape_data = data();

		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		if (shape_data == nullptr)
		{
			if (!ImGui::IsWindowHovered() or !insert_allowed_cursor()) return;

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				auto click_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
				auto ptr = std::make_shared<rectangle_shape>(click_pos, click_pos);
				set_data(ptr);
				shape_data = ptr;
				active_video_ = video_id;
			}
		}
		else
		{
			if (!ImGui::IsWindowFocused() or !insert_allowed_cursor()) return;

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				auto mouse_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
				shape_data->end = mouse_pos;

				const auto& tag = get_tag();
				ImRect draw_rect{ pos, pos + size };
				shape_data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), draw_rect, tag.fill_color(), tag.outline_color(), std::nullopt, false);
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				on_done();
			}
		}
	}

	void rectangle_tool::on_done()
	{
		if (!can_insert_region()) return;

		auto shape_data = data();
		if (!active_video_.has_value() or shape_data == nullptr)
		{
			reset();
			return;
		}

		auto[min_x, max_x] = std::minmax<int>(shape_data->start[0], shape_data->end[0]);
		auto[min_y, max_y] = std::minmax<int>(shape_data->start[1], shape_data->end[1]);

		shape_data->start = { min_x, min_y };
		shape_data->end = { max_x, max_y };

		if (shape_data->start != shape_data->end)
		{
			insert_region(*active_video_);
		}

		reset();
	}
}
