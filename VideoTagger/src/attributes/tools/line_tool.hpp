#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/line_shape.hpp>

namespace vt
{
	class line_tool : public shape_tool<line_shape>
	{
	public:
		line_tool(const tag& tag, const std::string& attribute_name) :
			shape_tool<line_shape>{ tag, attribute_name } {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
	};
}
