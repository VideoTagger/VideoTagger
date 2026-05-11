#pragma once
#include "generic_points_tool.hpp"
#include <attributes/shapes/polygon_shape.hpp>

namespace vt
{
	class polygon_tool : public generic_points_tool<polygon_shape>
	{
	public:
		polygon_tool(const tag& tag, const std::string& attribute_name) :
			generic_points_tool<polygon_shape>{ tag, attribute_name } {}
	};
}
