#pragma once
#include <memory>
#include <optional>
#include <utils/vec.hpp>
#include <impl/resettable.hpp>
#include <attributes/tools/impl/brush_tool.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/tools/extensions/impl/wand_tool_extension.hpp>
#include <attributes/tools/impl/rect_select_tool.hpp>
#include <attributes/shapes/points_shape.hpp>

namespace vt::ui
{
	enum class wand_sam2_mode : uint8_t
	{
		rectangle,
		points,
		mask,
	};

	struct wand_sam2_extension : public impl::wand_tool_extension, public vt::impl::resettable, public vt::impl::brush_tool, vt::impl::rect_select_tool
	{
	public:
		wand_sam2_extension(const std::string& name);

	private:
		wand_sam2_mode mode_;
		points_shape foreground_points_;
		points_shape background_points_;
		bool is_fg_point_;

	public:
		bool is_rect_mode() const;
		bool is_points_mode() const;
		bool is_mask_mode() const;

		void handle_point_selection(video_id_t video_id, ImRect draw_rect, const utils::vec2<int>& tex_size);

		void generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size, const std::optional<utils::vec4<float>>& rect = std::nullopt);
		void draw_remove_point_preview(const ImVec2& center, float brush_size);

		virtual void reset() override;

		virtual uint32_t property_column_count() const override;

		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override;
		virtual void render_properties() override;

		virtual void on_done() override;
		void on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size) override;
	};
}
