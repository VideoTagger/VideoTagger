#pragma once
#include <attributes/shapes/points_shape.hpp>

namespace vt
{
	class line_shape : public points_shape
	{
	public:
		line_shape() = default;
		line_shape(const std::vector<utils::vec2<uint32_t>>& points);
	};
}
