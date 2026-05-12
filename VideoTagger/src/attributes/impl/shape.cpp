#include "shape.hpp"

namespace vt::impl
{
	void shape::render(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius)
	{
		render_shape(shape_space, draw_min, draw_max, fill_color, outline_color);
		if (point_radius.has_value())
		{
			render_points(*point_radius, shape_space, draw_min, draw_max, 0xFFFFFFFF, 0xFFCCCCCC);
		}
	}
}
