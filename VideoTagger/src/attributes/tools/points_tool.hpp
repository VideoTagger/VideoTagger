#pragma once
#include "generic_points_tool.hpp"
#include <attributes/shapes/points_shape.hpp>

namespace vt
{
	class points_tool : public generic_points_tool<points_shape>
	{
	public:
		points_tool(const tag& tag, const std::string& attribute_name) :
			generic_points_tool<points_shape>{ tag, attribute_name } {}
	};
}
