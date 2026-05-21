#pragma once
#include "shape_tool.hpp"
#include <attributes/shapes/mask_shape.hpp>
#include <ui/toolbar/impl/toolbar_extensible_tool.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>

namespace vt
{
	class wand_tool : public shape_tool<mask_shape>, public ui::impl::toolbar_extensible_tool<ui::impl::wand_tool_extension>
	{
	public:
		wand_tool(const tag& tag, const std::string& attribute_name);

	public:
		virtual void on_activate() override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void on_done() override;

		virtual void render_properties() override;

		virtual void on_switch_extension(std::shared_ptr<ui::impl::wand_tool_extension> new_extension) override;
	};
}
