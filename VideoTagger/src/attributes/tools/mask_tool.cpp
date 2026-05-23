#include "mask_tool.hpp"
#include "pch.hpp"
#include <algorithm>
#include <utils/vec.hpp>
#include <core/app_context.hpp>
#include <utils/math.hpp>
#include <image/image_opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace vt
{
	mask_tool::mask_tool(const tag& tag, const std::string& attribute_name) : shape_tool<mask_shape>{ tag, attribute_name } {}

	uint32_t mask_tool::property_column_count() const
	{
		auto col_count = shape_tool<mask_shape>::property_column_count();
		col_count += brush_tool::property_column_count();
		return col_count;
	}

	void mask_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<mask_shape>::render_overlay(video_id, pos, size, tex_size);

		auto shape_data = data();

		//TODO: Move this somewhere outside
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_active_video = active_video_.has_value() and *active_video_ == video_id;
		bool insert_allowed = insert_allowed_cursor() and is_hovered;

		auto tex_size_int = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };

		const auto& io = ImGui::GetIO();

		auto mouse_pos = ImGui::GetMousePos();
		auto mpos = to_texture_space(mouse_pos, pos, size, tex_size);
		if (insert_allowed)
		{
			bool was_created = false;
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (shape_data == nullptr or !is_active_video)
				{
					auto ptr = std::make_shared<mask_shape>(tex_size_int[0], tex_size_int[1]);
					set_data(ptr);
					shape_data = ptr;
					was_created = true;
				}

				active_video_ = video_id;
				is_active_video = active_video_.has_value() and *active_video_ == video_id;
			}

			handle_drawing(shape_data, video_id, pos, size, tex_size, is_eraser() ? 0 : 255);

			if (was_created)
			{
				shape_data->recalculate_bounding_box();
			}
		}
		
		if (shape_data != nullptr and is_active_video)
		{
			const auto& tag = get_tag();
			ImRect draw_rect{ pos, pos + size };
			shape_data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), draw_rect, tag.fill_color(), tag.outline_color(), std::nullopt, false, video_id);

			if (ImGui::IsKeyPressed(ImGuiKey_Enter) and insert_allowed)
			{
				on_done();
			}
		}

		if (insert_allowed)
		{
			auto zoom_factor = size.x / tex_size.x;
			draw_brush_preview(mouse_pos, zoom_factor * brush_size());
		}
	}

	void mask_tool::on_done()
	{
		if (!can_insert_region()) return;

		auto shape_data = data();
		if (!active_video_.has_value() or shape_data == nullptr)
		{
			reset();
			return;
		}

		//TODO: Check if mask is not empty
		if (!shape_data->mask.empty())
		{
			insert_region(*active_video_);
		}
		reset();
	}

	void mask_tool::render_properties()
	{
		brush_tool::render_properties();
		shape_tool<mask_shape>::render_properties();
	}

	void mask_tool::draw_brush_preview(const ImVec2& center, float brush_size)
	{
		auto* draw_list = ImGui::GetWindowDrawList();
		const auto& tag = get_tag();
		auto outline_color = ctx_.current_theme.get_rgba(theme_color::tool_preview_outline);

		switch (get_brush_type())
		{
			case brush_type::circle:
			{
				draw_list->AddCircle(center, brush_size, outline_color);
				//draw_list->AddCircleFilled(center, brush_size, tag.fill_color());
				break;
			}
			case brush_type::square:
			{
				draw_list->AddRect({ center.x - brush_size, center.y - brush_size }, { center.x + brush_size, center.y + brush_size }, outline_color);
				//draw_list->AddRectFilled({ center.x - brush_size, center.y - brush_size }, { center.x + brush_size, center.y + brush_size }, tag.fill_color());
				break;
			}
			default: break;
		}
	}
}
