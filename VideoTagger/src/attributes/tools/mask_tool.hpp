#pragma once
#include <cstdint>
#include "shape_tool.hpp"
#include <attributes/tools/impl/brush_tool.hpp>
#include <attributes/shapes/mask_shape.hpp>

namespace vt
{
	enum class mask_tool_type : uint8_t
	{
		circle,
		square,
	};

	class mask_tool : public shape_tool<mask_shape>, public impl::brush_tool
	{
	public:
		mask_tool(const tag& tag, const std::string& attribute_name);

	public:
		virtual uint32_t property_column_count() const override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void on_done() override;

		virtual void render_properties() override;
	private:
		void draw_brush_preview(const ImVec2& center, float brush_size);
	};
}
