#include "shape.hpp"

namespace vt::impl
{
	float shape::scale_value(uint32_t value, uint32_t max_value, float draw_start, float draw_end)
	{
		return draw_start + static_cast<float>(value) / static_cast<float>(max_value) * (draw_end - draw_start);
	}
	ImVec2 shape::scale_point(utils::vec2<uint32_t> point, utils::vec2<uint32_t> point_space, ImRect draw_rect)
	{
		return { 
			scale_value(point[0], point_space[0], draw_rect.Min.x, draw_rect.Max.x),
			scale_value(point[1], point_space[1], draw_rect.Min.y, draw_rect.Max.y)
		};
	}

	utils::vec2<uint32_t>* shape::closest_point(utils::vec2<uint32_t> point, float max_distance)
	{
		return const_cast<utils::vec2<uint32_t>*>(static_cast<const shape*>(this)->closest_point(point, max_distance));
	}

	void shape::render(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color, std::optional<float> point_radius)
	{
		render_shape(shape_space, draw_rect, outline_color, fill_color);
		if (point_radius.has_value())
		{
			render_points(*point_radius, shape_space, draw_rect, 0xFFCCCCCC, 0xFFFFFFFF);
		}
	}
}
