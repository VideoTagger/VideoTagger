#pragma once
#include <memory>
#include <impl/resettable.hpp>
#include <attributes/tools/impl/brush_tool.hpp>
#include <attributes/tools/impl/points_tool.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>
#include <attributes/tools/impl/rect_select_tool.hpp>
#include <attributes/shapes/points_shape.hpp>

namespace vt::ui
{
	enum class wand_watershed_mode : uint8_t
	{
		points,
		mask,
	};

	struct wand_watershed_extension : public impl::wand_tool_extension, public vt::impl::resettable, public vt::impl::brush_tool, public vt::impl::points_tool
	{
	public:
		wand_watershed_extension(const std::string& name);

	private:
		wand_watershed_mode mode_;
		bool is_fg_brush_;

	public:
		bool is_points_mode() const;
		bool is_mask_mode() const;

		void generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size);

		virtual void reset() override;
		virtual void on_done() override;

		virtual uint32_t property_column_count() const override;

		void on_finish_selection(video_id_t video_id, const utils::vec2<int>& tex_size);

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void render_properties() override;
		virtual void on_finish_point_selection(video_id_t video_id, const utils::vec2<int>& tex_size) override;

	};
}
