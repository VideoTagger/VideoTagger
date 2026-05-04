#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/circle_shape.hpp>

namespace vt
{
	class circle_tool : public shape_tool<circle_shape>
	{
	public:
		circle_tool(const std::string& id, const std::string& icon, const std::string& tooltip, const tag& tag, const std::string& attribute_name) :
			shape_tool<circle_shape>{ id, icon, tooltip, tag, attribute_name } {}

	private:
		ImVec2 start_mouse_pos_{};

	public:
		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
	};
}
