#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/mask_shape.hpp>

namespace vt
{
	class wand_tool : public shape_tool<mask_shape>
	{
	public:
		wand_tool(const tag& tag, const std::string& attribute_name);

	public:
		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void on_done() override;

		virtual void render_properties() override;
	};
}
