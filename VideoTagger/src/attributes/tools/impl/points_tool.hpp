#pragma once
#include <unordered_map>
#include <attributes/shapes/points_shape.hpp>
#include <impl/resettable.hpp>

namespace vt::impl
{
	struct points_tool
	{
	public:
		points_tool(bool has_bg_points);

	private:
		points_shape foreground_points_;
		points_shape background_points_;
		bool has_bg_points_;

	public:
		void handle_point_selection(video_id_t video_id, ImRect draw_rect, const utils::vec2<int>& tex_size);
		void draw_remove_point_preview(const ImVec2& center, float brush_size);

		void reset();
		points_shape& fg_points();
		const points_shape& fg_points() const;
		points_shape& bg_points();
		const points_shape& bg_points() const;

		virtual void on_finish_point_selection(video_id_t video_id, const utils::vec2<int>& tex_size) = 0;
	};
}
