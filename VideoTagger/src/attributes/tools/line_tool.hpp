#pragma once
#include "generic_points_tool.hpp"
#include <attributes/shapes/line_shape.hpp>

namespace vt
{
	class line_tool : public generic_points_tool<line_shape>
	{
	public:
		line_tool(const tag& tag, const std::string& attribute_name) :
			generic_points_tool<line_shape>{ tag, attribute_name } {}
	};
}
