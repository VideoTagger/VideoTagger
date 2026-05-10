#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/polygon_shape.hpp>

namespace vt
{
	class polygon_tool : public shape_tool<polygon_shape>
	{
	public:
		polygon_tool(const tag& tag, const std::string& attribute_name) :
			shape_tool<polygon_shape>{ tag, attribute_name } {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
	};
}
