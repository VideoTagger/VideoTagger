#pragma once
#include <imgui.h>
#include <utils/vec.hpp>
#include <attributes/shapes/mask_shape.hpp>

namespace vt
{
	enum class brush_type : uint8_t
	{
		circle,
		square,
	};
}

namespace vt::impl
{
	struct brush_tool
	{
	public:
		brush_tool() = default;

	private:
		int brush_size_ = 5;
		brush_type brush_type_ = brush_type::circle;
		bool is_eraser_ = false;

	public:
		///@return True if the data was modified, false otherwise
		bool handle_drawing(std::shared_ptr<mask_shape> data, video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size, uint8_t color = 255);
		void apply_brush(std::shared_ptr<mask_shape> data, const utils::vec2<int>& center, const utils::vec2<int>& tex_size, uint8_t color = 255);
		void draw_brush_preview(const ImVec2& center, float brush_size);

		void set_brush_size(int size);
		int brush_size() const;
		void set_is_eraser(bool is_eraser);
		bool is_eraser() const;

		void set_brush_type(brush_type type);
		brush_type get_brush_type() const;
		size_t property_column_count() const;

		void render_properties();
	};
}
