#pragma once
#include <attributes/shapes/points_shape.hpp>

namespace vt
{
	class line_shape : public points_shape
	{
	public:
		line_shape() = default;
		line_shape(const std::vector<utils::vec2<uint32_t>>& points);

	public:
		virtual bool contains(utils::vec2<uint32_t> point) const override;

		virtual void render_shape(utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) override;
		virtual void render(utils::vec2<uint32_t> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius) override;
	};
}

namespace vt::math
{
	template<>
	inline line_shape shape_lerp<line_shape>(const line_shape& start, const line_shape& end, float alpha)
	{
		if (start.points.size() != end.points.size())
		{
			debug::panic("Can't interpolate between shapes with different number of points");
		}

		line_shape result{};
		result.points.reserve(start.points.size());
		for (size_t i = 0; i < start.points.size(); ++i)
		{
			result.points.push_back(math::lerp(start.points[i], end.points[i], alpha));
		}
		return result;
	}
}
