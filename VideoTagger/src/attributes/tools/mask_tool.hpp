#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/mask_shape.hpp>

namespace vt
{
	enum class mask_tool_type
	{
		circle,
		square,
	};

	class mask_tool : public shape_tool<mask_shape>
	{
	public:
		mask_tool(const tag& tag, const std::string& attribute_name);

	public:
		int brush_size_;
		mask_tool_type brush_type_;

	public:
		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void on_done() override;

		virtual void render_properties() override;
	private:
		void apply_brush(const utils::vec2<int>& center, const utils::vec2<int>& tex_size, bool is_eraser = false);
	};
}
