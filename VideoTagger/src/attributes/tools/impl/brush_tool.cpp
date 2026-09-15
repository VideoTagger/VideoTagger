#include "brush_tool.hpp"
#include <core/app_context.hpp>
#include <opencv2/imgproc.hpp>
#include <image/image_opencv.hpp>

namespace vt::impl
{
	bool brush_tool::handle_drawing(std::shared_ptr<mask_shape> data, video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size, uint8_t color)
	{
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		bool was_modified = false;

		const auto& io = ImGui::GetIO();
		auto tex_size_int = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };

		auto mouse_pos = ImGui::GetMousePos();
		auto mpos = to_texture_space(mouse_pos, pos, size, tex_size);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			apply_brush(data, mpos, tex_size_int, color);
			was_modified = true;
		}
		else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) and data != nullptr)
		{
			auto prev_mouse_pos = mouse_pos - io.MouseDelta;
			auto prev_mpos = to_texture_space(prev_mouse_pos, pos, size, tex_size);

			cv::Point current_pt(mpos[0], mpos[1]);
			cv::Point prev_pt(prev_mpos[0], prev_mpos[1]);

			double distance = cv::norm(current_pt - prev_pt);

			int steps = std::max(1, static_cast<int>(distance));

			for (int i = 0; i <= steps; ++i)
			{
				double t = static_cast<double>(i) / steps;
				auto x = math::lerp(prev_pt.x, current_pt.x, t);
				auto y = math::lerp(prev_pt.y, current_pt.y, t);

				apply_brush(data, { x, y }, tex_size_int, color);
			}
		}
		return was_modified;
	}

	void brush_tool::apply_brush(std::shared_ptr<mask_shape> data, const utils::vec2<int>& center, const utils::vec2<int>& tex_size, uint8_t color)
	{
		auto mat = image_to_cvmat(data->mask);
		bool update_bb = false;

		utils::vec4<int> brush_area{ center - brush_size_, center + brush_size_ };

		switch (brush_type_)
		{
			case brush_type::circle:
			{
				cv::circle(mat, cv::Point(center[0], center[1]), brush_size_, cv::Scalar(color), cv::FILLED);
				update_bb = true;
			}
			break;
			case brush_type::square:
			{
				cv::rectangle(mat, cv::Point(center[0] - brush_size_, center[1] - brush_size_), cv::Point(center[0] + brush_size_, center[1] + brush_size_), cv::Scalar(color), cv::FILLED);
				update_bb = true;
			}
			break;
			default: break;
		}
		if (update_bb)
		{
			data->recalculate_bounding_box(brush_area, color != 0);
		}
	}

	void brush_tool::draw_brush_preview(const ImVec2& center, float brush_size)
	{
		auto* draw_list = ImGui::GetWindowDrawList();
		auto outline_color = ctx_.current_theme.get_rgba(theme_color::tool_preview_outline);

		switch (brush_type_)
		{
			case brush_type::circle:
			{
				draw_list->AddCircle(center, brush_size, outline_color);
				break;
			}
			case brush_type::square:
			{
				draw_list->AddRect({ center.x - brush_size, center.y - brush_size }, { center.x + brush_size, center.y + brush_size }, outline_color);
				break;
			}
			default: break;
		}
	}

	void brush_tool::set_brush_size(int size)
	{
		brush_size_ = size;
	}

	int brush_tool::brush_size() const
	{
		return brush_size_;
	}

	void brush_tool::set_is_eraser(bool is_eraser)
	{
		is_eraser_ = is_eraser;
	}

	bool brush_tool::is_eraser() const
	{
		return is_eraser_;
	}

	void brush_tool::set_brush_type(brush_type type)
	{
		brush_type_ = type;
	}

	brush_type brush_tool::get_brush_type() const
	{
		return brush_type_;
	}

	size_t brush_tool::property_column_count() const
	{
		return 3;
	}

	void brush_tool::render_properties()
	{
		const auto& style = ImGui::GetStyle();
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_brush, !is_eraser()))
			{
				set_is_eraser(false);
			}
			ui::tooltip("Brush");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_eraser, is_eraser()))
			{
				set_is_eraser(true);
			}
			ui::tooltip("Eraser");
			ImGui::PopStyleVar();
		}

		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			auto btype = get_brush_type();
			if (ui::icon_toggle_button(icons::shape_circle, btype == brush_type::circle))
			{
				set_brush_type(brush_type::circle);
			}
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::shape_rectangle, btype == brush_type::square))
			{
				set_brush_type(brush_type::square);
			}
			ImGui::PopStyleVar();
		}

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("100px").x + style.FramePadding.x * 2.f);
		int bsize = brush_size();
		if (ImGui::DragInt("##Brush Size", &bsize, 1, 1, 100, "%dpx", ImGuiSliderFlags_AlwaysClamp))
		{
			set_brush_size(bsize);
		}
		ui::tooltip("Brush size");
	}
}
