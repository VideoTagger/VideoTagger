#include "shape.hpp"
#include <core/app_context.hpp>

namespace vt::impl
{
	void shape::render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box, std::optional<video_id_t> video_id)
	{
		render_shape(shape_space, draw_rect, fill_color, outline_color, video_id);
		if (draw_bounding_box)
		{
			render_bounding_box(shape_space, draw_rect, fill_color, outline_color, video_id);
		}
		if (point_radius.has_value())
		{
			render_points(*point_radius, shape_space, draw_rect, ctx_.current_theme.get_rgba(theme_color::gizmo_point), ctx_.current_theme.get_rgba(theme_color::gizmo_point_outline), video_id);
			//render_points(*point_radius, shape_space, draw_rect, outline_color, outline_color);
		}
	}
}
