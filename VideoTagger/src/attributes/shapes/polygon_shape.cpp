#include "polygon_shape.hpp"

#include <utils/intersection.hpp>

namespace vt
{
	polygon_shape::polygon_shape(const std::vector<utils::vec2<int>>& vertices) : points_shape{ vertices } {}

	bool polygon_shape::contains(utils::vec2<int> point) const
	{
		std::vector<ImVec2> ps(points.size());
		for (size_t i = 0; i < ps.size(); ++i)
		{
			ps[i] = ImVec2(points[i][0], points[i][1]);
		}

		return utils::intersection::is_in_polygon(ImVec2(point[0], point[1]), ps);
	}

	void polygon_shape::render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();

		std::vector<ImVec2> ps(points.size());
		for (size_t i = 0; i < ps.size(); ++i)
		{
			ps[i] = math::scale_vec2(points[i], utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);
		}

		if (utils::intersection::is_convex_polygon(ps))
		{
			draw_list->AddConvexPolyFilled(ps.data(), ps.size(), fill_color);
		}
		else
		{
			draw_list->AddConcavePolyFilled(ps.data(), ps.size(), fill_color);
		}
		draw_list->AddPolyline(ps.data(), ps.size(), outline_color, ImDrawFlags_Closed, 1.f);
	}

	void polygon_shape::render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box)
	{
		shape::render(shape_space, draw_rect, fill_color, outline_color, point_radius, draw_bounding_box);
	}
}
