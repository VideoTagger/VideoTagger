#include "line_shape.hpp"

#include <utils/intersection.hpp>

namespace vt
{
	line_shape::line_shape(const std::vector<utils::vec2<uint32_t>>& points) : points_shape{ points } {}

	bool line_shape::contains(utils::vec2<uint32_t> point) const
	{
		auto p = ImVec2(point[0], point[1]);

		for (size_t i =	1; i < points.size(); ++i)
		{
			auto p1 = points[i - 1];
			auto p2 = points[i];
			if (utils::intersection::is_on_line(p, ImVec2(p1[0], p1[1]), ImVec2(p2[0], p2[1]))) return true;
		}

		return false;
	}

	void line_shape::render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();

		for (size_t i = 1; i < points.size(); ++i)
		{
			auto scaled_p1 = impl::shape::scale_point(points[i - 1], shape_space, draw_rect);
			auto scaled_p2 = impl::shape::scale_point(points[i], shape_space, draw_rect);

			draw_list->AddLine(scaled_p1, scaled_p2, outline_color);
		}
	}
}
