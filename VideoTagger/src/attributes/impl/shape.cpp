#include "shape.hpp"

namespace vt::impl
{
	utils::vec2<uint32_t>* shape::closest_point(utils::vec2<uint32_t> point, float max_distance)
	{
		return const_cast<utils::vec2<uint32_t>*>(static_cast<const shape*>(this)->closest_point(point, max_distance));
	}

	void shape::render(utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius)
	{
		render_shape(shape_space, draw_min, draw_max, fill_color, outline_color);
		if (point_radius.has_value())
		{
			render_points(*point_radius, shape_space, draw_min, draw_max, 0xFFFFFFFF, 0xFFCCCCCC);
		}
	}
}
