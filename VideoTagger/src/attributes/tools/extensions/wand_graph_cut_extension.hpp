#pragma once
#include <memory>
#include <impl/resettable.hpp>
#include <attributes/tools/impl/brush_tool.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>

namespace vt::ui
{
	enum class wand_graph_cut_mode : uint8_t
	{
		rectangle,
		mask,
	};

	struct wand_graph_cut_extension : public impl::wand_tool_extension, public vt::impl::resettable, public vt::impl::brush_tool
	{
	public:
		wand_graph_cut_extension();

	private:
		std::unique_ptr<rectangle_shape> rect_data_;
		wand_graph_cut_mode mode_;
		bool is_fg_brush_;

	public:
		bool is_rect_mode() const;
		bool is_mask_mode() const;

		void generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size);

		virtual void reset() override;

		virtual uint32_t property_column_count() const override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void render_properties() override;
	};
}
