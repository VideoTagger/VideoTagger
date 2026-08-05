#pragma once
#include "shape_tool.hpp"
#include <ui/widgets/combo.hpp>
#include <attributes/shapes/mask_shape.hpp>
#include <ui/toolbar/impl/toolbar_extensible_tool.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>

namespace vt
{
	class wand_tool : public shape_tool<mask_shape>, public ui::impl::toolbar_extensible_tool<ui::impl::wand_tool_extension>
	{
	public:
		wand_tool(const tag& tag, const std::string& attribute_name);

	private:
		ui::combo<std::string> extension_combo_;

	public:
		virtual void on_activate() override;
		virtual void on_switch_context() override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void on_done() override;

		virtual uint32_t property_column_count() const override;

		virtual void render_properties() override;
		virtual void render_popup_body(ui::widget_list& widgets, ui::button_bar<int>& button_bar);

		virtual void on_switch_extension(std::shared_ptr<ui::impl::wand_tool_extension> new_extension) override;
	private:
		std::vector<std::string> extension_names() const;
	};
}
