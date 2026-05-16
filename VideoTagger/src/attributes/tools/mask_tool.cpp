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
	mask_tool::mask_tool(const tag& tag, const std::string& attribute_name) : shape_tool<mask_shape>{ tag, attribute_name }, brush_size_{ 5 }, brush_type_{ mask_tool_type::circle }
	{
		auto col_count = shape_tool<mask_shape>::property_column_count();
		set_property_column_count(col_count + 2);
	}

	void mask_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<mask_shape>::render_overlay(video_id, pos, size, tex_size);

		auto& shape_data = data();

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
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and insert_allowed)
		{
			if (!shape_data.has_value())
			{
				shape_data = mask_shape{ tex_size_int[0], tex_size_int[1] };
			}

			apply_brush(mpos, tex_size_int);
			active_video_ = video_id;
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) and insert_allowed and shape_data.has_value())
		{
			auto prev_mouse_pos = mouse_pos - io.MouseDelta;
			auto prev_mpos = to_texture_space(prev_mouse_pos, pos, size, tex_size);

			cv::Point current_pt(mpos[0], mpos[1]);
			cv::Point prev_pt(prev_mpos[0], prev_mpos[1]);

			double distance = cv::norm(current_pt - prev_pt);

			// Determine how many stamps to place (e.g., one per pixel of movement)
			int steps = std::max(1, static_cast<int>(distance));

			// Interpolate and stamp the brush along the path
			for (int i = 0; i <= steps; ++i)
			{
				double t = static_cast<double>(i) / steps;
				auto x = math::lerp(prev_pt.x, current_pt.x, t);
				auto y = math::lerp(prev_pt.y, current_pt.y, t);

				apply_brush({ x, y }, tex_size_int);
			}
		}

		if (shape_data.has_value() and is_active_video)
		{
			const auto& tag = get_tag();
			shape_data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), pos, pos + size, tag.fill_color(), tag.outline_color(), std::nullopt);

			if (ImGui::IsKeyPressed(ImGuiKey_Enter) and insert_allowed)
			{
				on_done();
			}
		}
	}

	void mask_tool::on_done()
	{
		if (!can_insert_region()) return;

		auto& shape_data = data();
		if (!active_video_.has_value() or !shape_data.has_value())
		{
			reset();
			return;
		}

		//TODO: Check if mask is not empty
		if (true)
		{
			insert_region(*active_video_);
		}
		reset();
	}

	void mask_tool::render_properties()
	{
		const auto& style = ImGui::GetStyle();
		ImGui::TableNextColumn();
		if (ui::icon_toggle_button(icons::shape_circle, brush_type_ == mask_tool_type::circle))
		{
			brush_type_ = mask_tool_type::circle;
		}
		ImGui::SameLine();
		if (ui::icon_toggle_button(icons::shape_rectangle, brush_type_ == mask_tool_type::square))
		{
			brush_type_ = mask_tool_type::square;
		}

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("100px").x + style.FramePadding.x * 2.f);
		ImGui::DragInt("##Brush Size", &brush_size_, 1, 1, 100, "%dpx", ImGuiSliderFlags_AlwaysClamp);
		ui::tooltip("Brush size");

		shape_tool<mask_shape>::render_properties();
	}

	void mask_tool::apply_brush(const utils::vec2<int>& center, const utils::vec2<int>& tex_size, bool is_eraser)
	{
		auto color = is_eraser ? cv::Scalar(0) : cv::Scalar(255);
		auto mat = image_to_cvmat(data()->mask_);

		switch (brush_type_)
		{
			case mask_tool_type::circle:
			{
				cv::circle(mat, cv::Point(center[0], center[1]), brush_size_, color, cv::FILLED);
				break;
			}
			case mask_tool_type::square:
			{
				cv::rectangle(mat, cv::Point(center[0] - brush_size_, center[1] - brush_size_), cv::Point(center[0] + brush_size_, center[1] + brush_size_), color, cv::FILLED);
				break;
			}
			default: break;
		}
	}
}
