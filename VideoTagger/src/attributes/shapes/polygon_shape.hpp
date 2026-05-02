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

namespace vt::math
{
	template<>
	inline polygon_shape shape_lerp<polygon_shape>(const polygon_shape& start, const polygon_shape& end, float alpha)
	{
		if (start.points.size() != end.points.size())
		{
			debug::panic("Can't interpolate between shapes with different number of points");
		}

		polygon_shape result{};
		result.points.reserve(start.points.size());
		for (size_t i = 0; i < start.points.size(); ++i)
		{
			result.points.push_back(math::lerp(start.points[i], end.points[i], alpha));
		}
		return result;
	}
}
