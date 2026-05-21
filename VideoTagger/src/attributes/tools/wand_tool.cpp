#include "wand_tool.hpp"

namespace vt
{
	wand_tool::wand_tool(const tag& tag, const std::string& attribute_name) : shape_tool<mask_shape>{ tag, attribute_name } {}

	void wand_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{

	}

	void wand_tool::on_done()
	{

	}

	void wand_tool::render_properties()
	{

	}
}

