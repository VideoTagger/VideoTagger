#pragma once
#include <attributes/shapes/points_shape.hpp>

namespace vt
{
	class polygon_shape : public points_shape
	{
	public:
		polygon_shape() = default;
		polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices);

	public:
		virtual bool contains(utils::vec2<uint32_t> point) const override;

		virtual void render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color) override;
	};
}
