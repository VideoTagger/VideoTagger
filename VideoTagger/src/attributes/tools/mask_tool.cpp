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

	void mask_tool::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		shape_tool<mask_shape>::render_overlay(video_id, pos, size, tex_size);

		auto& shape_data = data();

		//TODO: Move this somewhere outside
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		const cv::Scalar draw_color{ 255 };
		int brush_size = 5;

		auto tex_size_int = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };
		if (!shape_data.has_value())
		{
			if (!ImGui::IsWindowHovered() or !insert_allowed_cursor()) return;

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				auto mouse_pos = ImGui::GetMousePos();
				auto mpos = to_texture_space(mouse_pos, pos, size, tex_size);

				shape_data = mask_shape{ tex_size_int[0], tex_size_int[1] };

				auto mat = image_to_cvmat(shape_data->mask_);
				cv::circle(mat, cv::Point(mpos[0], mpos[1]), brush_size, draw_color, cv::FILLED);
				active_video_ = video_id;
			}
		}
		else
		{
			if (!ImGui::IsWindowFocused() or !insert_allowed_cursor()) return;

			const auto& io = ImGui::GetIO();

			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				auto mouse_pos = ImGui::GetMousePos();
				auto prev_mouse_pos = ImVec2(mouse_pos.x - io.MouseDelta.x, mouse_pos.y - io.MouseDelta.y);

				auto mpos = to_texture_space(mouse_pos, pos, size, tex_size);
				auto prev_mpos = to_texture_space(prev_mouse_pos, pos, size, tex_size);

				auto mat = image_to_cvmat(shape_data->mask_);
				cv::Point current_pt(mpos[0], mpos[1]);
				cv::Point prev_pt(prev_mpos[0], prev_mpos[1]);
				//cv::circle(mat, cv::Point(mpos[0], mpos[1]), brush_size, draw_color), cv::FILLED);

				double distance = cv::norm(current_pt - prev_pt);

				// Determine how many stamps to place (e.g., one per pixel of movement)
				int steps = std::max(1, static_cast<int>(distance));

				
				// Interpolate and stamp the brush along the path
				for (int i = 0; i <= steps; ++i)
				{
					double t = static_cast<double>(i) / steps;
					auto x = math::lerp(prev_pt.x, current_pt.x, t);
					auto y = math::lerp(prev_pt.y, current_pt.y, t);

					cv::circle(mat, cv::Point(x, y), brush_size, draw_color, cv::FILLED);
				}
			}

			const auto& tag = get_tag();
			shape_data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), pos, pos + size, tag.fill_color(), tag.outline_color(), std::nullopt);
		}
	}

	void mask_tool::on_done()
	{
		if (!can_insert_region()) return;

		auto& shape_data = data();
		//if (!active_video_.has_value() or !shape_data.has_value())
		//{
		//	reset();
		//	return;
		//}

		//if (shape_data->radius != 0)
		//{
		//	insert_region(*active_video_);
		//}
		reset();
	}
}
