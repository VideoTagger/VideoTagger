#pragma once
#include <tags/shapes/points_shape.hpp>

namespace vt
{
	class polygon_shape : public points_shape
	{
		polygon_shape() = default;
		polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices);
	};
}
