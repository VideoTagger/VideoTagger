#include "line_shape.hpp"

#include <utils/intersection.hpp>

namespace vt
{
	line_shape::line_shape(const std::vector<utils::vec2<int>>& points) : points_shape{ points } {}

	bool line_shape::contains(utils::vec2<int> point, float added_radius) const
	{
		if (points_shape::contains(point, added_radius)) return true;

		auto p = ImVec2(point[0], point[1]);

		for (size_t i =	1; i < points.size(); ++i)
		{
			auto p1 = points[i - 1];
			auto p2 = points[i];
			if (utils::intersection::is_on_line(p, ImVec2(p1[0], p1[1]), ImVec2(p2[0], p2[1]))) return true;
		}

		return false;
	}

	void line_shape::render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id)
	{
		auto draw_list = ImGui::GetWindowDrawList();

		for (size_t i = 1; i < points.size(); ++i)
		{
			auto scaled_p1 = math::scale_vec2(points[i - 1], utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);
			auto scaled_p2 = math::scale_vec2(points[i], utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);

			draw_list->AddLine(scaled_p1, scaled_p2, outline_color);
		}
	}

	void line_shape::render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box, std::optional<video_id_t> video_id)
	{
		shape::render(shape_space, draw_rect, fill_color, outline_color, point_radius, draw_bounding_box, video_id);
	}
}
