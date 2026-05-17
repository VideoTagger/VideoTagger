#include "shape.hpp"

namespace vt::impl
{
	void shape::render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box)
	{
		render_shape(shape_space, draw_rect, fill_color, outline_color);
		if (draw_bounding_box)
		{
			render_bounding_box(shape_space, draw_rect, fill_color, outline_color);
		}
		if (point_radius.has_value())
		{
			render_points(*point_radius, shape_space, draw_rect, 0xFFFFFFFF, 0xFFCCCCCC);
			//render_points(*point_radius, shape_space, draw_rect, outline_color, outline_color);
		}
	}
}
