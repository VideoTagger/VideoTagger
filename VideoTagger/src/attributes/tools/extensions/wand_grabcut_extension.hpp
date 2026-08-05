#pragma once
#include <memory>
#include <impl/resettable.hpp>
#include <attributes/tools/impl/brush_tool.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>
#include <attributes/tools/impl/rect_select_tool.hpp>

namespace vt::ui
{
	enum class wand_grabcut_mode : uint8_t
	{
		rectangle,
		mask,
	};

	struct wand_grabcut_extension : public impl::wand_tool_extension, public vt::impl::resettable, public vt::impl::brush_tool, vt::impl::rect_select_tool
	{
	public:
		wand_grabcut_extension(const std::string& name);

	private:
		wand_grabcut_mode mode_;
		bool is_fg_brush_;

	public:
		bool is_rect_mode() const;
		bool is_mask_mode() const;

		void generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size);

		virtual void reset() override;

		virtual uint32_t property_column_count() const override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void render_properties() override;

		void on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size);
	};
}
