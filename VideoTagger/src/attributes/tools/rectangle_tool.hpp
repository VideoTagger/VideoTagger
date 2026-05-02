#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/rectangle_shape.hpp>

namespace vt
{
	class rectangle_tool : public shape_tool<rectangle_shape>
	{
	public:
		rectangle_tool(const std::string& id, const std::string& icon, const std::string& tooltip, const tag& tag) : shape_tool<rectangle_shape>{ id, icon, tooltip, tag } {}

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
	};
}
