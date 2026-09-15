#include "points_tool.hpp"
#include <core/app_context.hpp>

namespace vt::impl
{
	points_tool::points_tool(bool has_bg_points) : has_bg_points_{ has_bg_points } {}

	void points_tool::handle_point_selection(video_id_t video_id, ImRect draw_rect, const utils::vec2<int>& tex_size)
	{
		static auto to_texture_space = [](const ImVec2& screen_pos, ImRect draw_rect, const utils::vec2<int>& tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, draw_rect.Min, draw_rect.Max, utils::vec2<int>{}, tex_size, false);
		};

		auto& io = ImGui::GetIO();

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_focused = ImGui::IsWindowFocused();

		bool is_mouse_left_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool is_mouse_right_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right) and has_bg_points_;
		bool is_mouse_clicked = is_mouse_left_clicked or is_mouse_right_clicked;
		bool is_ctrl_held = io.KeyCtrl;
		auto mouse_pos = to_texture_space(ImGui::GetMousePos(), draw_rect, tex_size);

		const auto remove_point_brush_size = 10.f;
		const auto point_add_brush_size = 5.f;

		if (is_hovered and is_mouse_clicked)
		{
			bool needs_update = false;
			if (is_ctrl_held)
			{
				for (auto& current_points_shape : (!has_bg_points_ ? std::initializer_list{ &foreground_points_ } : std::initializer_list{ &foreground_points_, &background_points_ }))
				{
					auto closest_point = current_points_shape->closest_point(mouse_pos, remove_point_brush_size);
					if (closest_point != nullptr)
					{
						ptrdiff_t idx = closest_point - current_points_shape->points.data();
						current_points_shape->points.erase(current_points_shape->points.begin() + idx);
						needs_update = true;
						break;
					}
				}
			}
			else
			{
				auto& current_points_shape = (!has_bg_points_ or is_mouse_left_clicked) ? foreground_points_ : background_points_;
				auto closest_point = current_points_shape.closest_point(mouse_pos, point_add_brush_size);
				if (closest_point == nullptr)
				{
					current_points_shape.points.push_back(mouse_pos);
					needs_update = true;
				}
			}

			if (needs_update)
			{
				on_finish_point_selection(video_id, tex_size);
			}
		}

		if (is_focused)
		{
			foreground_points_.render_points(3.f, tex_size, draw_rect, IM_COL32(0, 255, 0, 255), IM_COL32(0, 255, 0, 127));
			background_points_.render_points(3.f, tex_size, draw_rect, IM_COL32(255, 0, 0, 255), IM_COL32(255, 0, 0, 127));
		}

		if (is_hovered and is_ctrl_held)
		{
			auto zoom_factor = draw_rect.GetWidth() / (float)tex_size.x();
			draw_remove_point_preview(ImGui::GetMousePos(), zoom_factor * remove_point_brush_size);
		}
	}

	void points_tool::draw_remove_point_preview(const ImVec2& center, float brush_size)
	{
		auto* draw_list = ImGui::GetWindowDrawList();
		auto outline_color = ctx_.current_theme.get_rgba(theme_color::tool_preview_outline);

		auto half_brush_size = brush_size / 2.f;
		draw_list->AddLine({ center.x - half_brush_size, center.y - half_brush_size }, { center.x + half_brush_size, center.y + half_brush_size }, IM_COL32(255, 0, 0, 255), 1.f);
		draw_list->AddLine({ center.x - half_brush_size, center.y + half_brush_size }, { center.x + half_brush_size, center.y - half_brush_size }, IM_COL32(255, 0, 0, 255), 1.f);
	}

	void points_tool::reset()
	{
		foreground_points_.points.clear();
		background_points_.points.clear();
	}

	points_shape& points_tool::fg_points()
	{
		return foreground_points_;
	}

	const points_shape& points_tool::fg_points() const
	{
		return foreground_points_;
	}

	points_shape& points_tool::bg_points()
	{
		return background_points_;
	}

	const points_shape& points_tool::bg_points() const
	{
		return background_points_;
	}
}
