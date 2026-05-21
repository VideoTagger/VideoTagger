#include "wand_graph_cut_extension.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>
#include <image/image_opencv.hpp>
#include <opencv2/highgui.hpp>

namespace vt::ui
{
	wand_graph_cut_extension::wand_graph_cut_extension()
	{
		set_property_column_count(wand_tool_extension::property_column_count() + 1);
	}

	bool wand_graph_cut_extension::is_rect_mode() const
	{
		return std::holds_alternative<std::unique_ptr<rectangle_shape>>(mode_data_);
	}

	bool wand_graph_cut_extension::is_mask_mode() const
	{
		return std::holds_alternative<std::unique_ptr<mask_shape>>(mode_data_);
	}

	void wand_graph_cut_extension::generate_mask(video_id_t video_id)
	{
		auto vid_it = ctx_.displayed_videos.find(video_id);
		if (vid_it == ctx_.displayed_videos.end()) return;
		auto& vid_data = *vid_it;
		
		if (is_rect_mode())
		{
			const auto& mode_data = std::get<std::unique_ptr<rectangle_shape>>(mode_data_);
			if (mode_data->start == mode_data->end) return;

			ctx_.tasks.run([this, &vid_data, start = mode_data->start, end = mode_data->end]()
			{
				set_busy(true);

				auto& stream = vid_data.video;
				image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
				if (stream.update_from_current_frame(img))
				{
					auto bgr_img = img.convert<image_pixel_format::bgr8>([](const image_pixel_format::rgb8& pixel)
					{
						return image_pixel_format::bgr8{ pixel.b, pixel.g, pixel.r };
					});
					auto cv_img = image_to_cvmat(bgr_img);
					cv::Rect cv_rect{ cv::Point2i{ start.x(), start.y() }, cv::Point2i{ end.x(), end.y() } };

					cv::Mat mask;
					cv::Mat model_fg;
					cv::Mat model_bg;

					int iterations = 1;
					cv::grabCut(cv_img, mask, cv_rect, model_bg, model_fg, iterations, cv::GC_INIT_WITH_RECT);
					cv::Mat result_mask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

					//cv::Mat foreground(cv_img.size(), CV_8UC3, cv::Scalar(0, 0, 0));
					//cv_img.copyTo(foreground, result_mask);

					auto mask_data = data();
					if (mask_data == nullptr)
					{
						debug::error("Mask data is null, while generating graph cut mask");
					}
					else
					{
						mask_data->mask.allocate(result_mask.cols, result_mask.rows);
						auto cv_mask_data = image_to_cvmat(mask_data->mask);
						result_mask.copyTo(cv_mask_data);
					}

					set_busy(false);

					//cv::imshow("Original", cv_img);
					//cv::imshow("Mask", result_mask);
					//cv::imshow("Foreground", foreground);
				}
			})
			.then(ctx_.tasks.on_main(), [this]()
			{
				reset();
			});
		}
		//reset();
	}

	void wand_graph_cut_extension::reset()
	{
		std::visit([](auto& data)
		{
			data.reset();
		}, mode_data_);
	}

	void wand_graph_cut_extension::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		if (is_busy()) return;

		if (is_rect_mode())
		{
			auto& data = std::get<std::unique_ptr<rectangle_shape>>(mode_data_);
			bool is_hovered = ImGui::IsWindowHovered();
			bool is_focused = ImGui::IsWindowFocused();

			if (data == nullptr and is_hovered and ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				auto click_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
				data = std::make_unique<rectangle_shape>(click_pos, click_pos);
			}
			else if (data != nullptr and is_focused)
			{
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
				{
					auto mouse_pos = to_texture_space(ImGui::GetMousePos(), pos, size, tex_size);
					data->end = mouse_pos;

					ImRect draw_rect{ pos, pos + size };
					data->render(utils::vec2<int>({ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }), draw_rect, 0, IM_COL32(0, 0xFF, 0, 0xFF), std::nullopt, false);
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					generate_mask(video_id);
				}
			}
		}
	}

	void wand_graph_cut_extension::render_properties()
	{
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_rect_selector, is_rect_mode()))
			{
				
			}
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_mask, is_mask_mode()))
			{
				
			}
			ImGui::PopStyleVar();
		}
		wand_tool_extension::render_properties();
	}
}
