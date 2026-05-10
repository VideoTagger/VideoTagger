#include "pch.hpp"
#include <algorithm>
#include "polygon_tool.hpp"
#include <utils/vec.hpp>
#include <core/app_context.hpp>
#include <utils/math.hpp>
#include <utils/intersection.hpp>

namespace vt
{
	void polygon_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<polygon_shape>::render_overlay(video_id, pos, size, tex_size);

		auto& shape_data = data();

		//TODO: Move this somewhere outside
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<uint32_t>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<uint32_t>{}, utils::vec2<uint32_t>{ static_cast<uint32_t>(tex_size.x), static_cast<uint32_t>(tex_size.y) });
		};

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (!ImGui::IsWindowHovered() or !insert_allowed_cursor()) return;

			auto mouse_pos = ImGui::GetMousePos();
			if (utils::intersection::is_in_rect(mouse_pos, ImRect{ pos, pos + size }))
			{
				if (!shape_data.has_value())
				{
					shape_data.emplace();
				}

				shape_data->points.push_back(to_texture_space(mouse_pos, pos, size, tex_size));
				active_video_ = video_id;
			}
		}

		if (!active_video_.has_value() or *active_video_ != video_id) return;

		if (shape_data.has_value())
		{
			const auto& tag = get_tag();
			shape_data->render(utils::vec2<uint32_t>({ (uint32_t)tex_size.x, (uint32_t)tex_size.y }), pos, pos + size, tag.fill_color(), tag.outline_color(), 3.f);

			if (ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				if (!shape_data->points.empty())
				{
					insert_region(video_id);
				}
				reset();
			}

		}
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			reset();
		}
	}
}
