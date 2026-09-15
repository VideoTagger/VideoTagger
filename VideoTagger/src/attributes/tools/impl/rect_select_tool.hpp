#pragma once
#include <memory>
#include <impl/resettable.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <core/types.hpp>
#include <utils/vec.hpp>

namespace vt::impl
{
	struct rect_select_tool
	{
	public:
		rect_select_tool() = default;

	private:
		std::unique_ptr<rectangle_shape> rect_data_;

	public:
		void handle_rect_selection(video_id_t video_id, ImRect draw_rect, const utils::vec2<int>& tex_size);

		std::unique_ptr<rectangle_shape>& rect_select_data();
		const std::unique_ptr<rectangle_shape>& rect_select_data() const;

		virtual void reset();
		virtual void on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size) = 0;
	};
}
