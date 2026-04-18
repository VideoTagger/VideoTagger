#include "polygon_shape.hpp"

namespace vt
{
	polygon_shape::polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices) : points_shape{ vertices } {}
}
