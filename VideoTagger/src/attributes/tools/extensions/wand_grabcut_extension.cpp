#include "wand_grabcut_extension.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>
#include <image/image_opencv.hpp>
#include <opencv2/highgui.hpp>

namespace vt::ui
{
	wand_grabcut_extension::wand_grabcut_extension(const std::string& name) : impl::wand_tool_extension{ name }, mode_{ wand_grabcut_mode::rectangle }, is_fg_brush_{ true } {}

	bool wand_grabcut_extension::is_rect_mode() const
	{
		return mode_ == wand_grabcut_mode::rectangle;
	}

	bool wand_grabcut_extension::is_mask_mode() const
	{
		return mode_ == wand_grabcut_mode::mask;
	}

	void wand_grabcut_extension::generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size)
	{
		auto vid_it = ctx_.displayed_videos.find(video_id);
		if (vid_it == ctx_.displayed_videos.end()) return;
		auto& vid_data = *vid_it;
		
		switch (mode_)
		{
			case wand_grabcut_mode::rectangle:
			{
				auto& rect_data = rect_select_data();
				if (rect_data == nullptr or rect_data->start == rect_data->end) return;

				ctx_.tasks.run([this, &vid_data, start = rect_data->start.max({ 0, 0 }), end = rect_data->end.min(tex_size)]()
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
					rect_select_tool::reset();
				});
			}
			break;
			case wand_grabcut_mode::mask:
			{
				ctx_.tasks.run([this, &vid_data]()
				{
					auto mask_data = data();
					if (mask_data == nullptr)
					{
						debug::error("Mask data is null, while generating graph cut mask");
						return;
					}

					set_busy(true);

					auto& stream = vid_data.video;
					image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
					if (stream.update_from_current_frame(img))
					{
						auto bgr_img = img.convert<image_pixel_format::bgr8>([](const image_pixel_format::rgb8& pixel)
						{
							auto gray = pixel.r;
							return image_pixel_format::bgr8{ pixel.b, pixel.g, gray };
						});
						auto cv_img = image_to_cvmat(bgr_img);
						cv::Rect cv_rect;

						cv::Mat mask = image_to_cvmat(mask_data->mask).clone();
						for (int y = 0; y < mask.rows; ++y)
						{
							for (int x = 0; x < mask.cols; ++x)
							{
								auto& pixel = mask.at<uint8_t>(y, x);
								if (pixel == 0xFF)
								{
									pixel = cv::GC_FGD;
								}
							}
						}
						cv::Mat model_fg;
						cv::Mat model_bg;

						int iterations = 1;
						cv::grabCut(cv_img, mask, cv_rect, model_bg, model_fg, iterations, cv::GC_INIT_WITH_MASK);
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
					//rect_data_.reset();
				});
			}
			break;
		}
		//reset();
	}

	void wand_grabcut_extension::reset()
	{
		mode_ = wand_grabcut_mode::rectangle;
		rect_select_tool::reset();
		set_data(nullptr);
	}

	void wand_grabcut_extension::on_done()
	{
		rect_select_tool::reset();
	}

	uint32_t wand_grabcut_extension::property_column_count() const
	{
		auto col_count = 1;
		if (is_mask_mode())
		{
			col_count += brush_tool::property_column_count();
		}
		return col_count;
	}

	void wand_grabcut_extension::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		static auto to_texture_space = [](const ImVec2& screen_pos, ImVec2 pos, ImVec2 size, ImVec2 tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
		};

		if (is_busy()) return;

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_focused = ImGui::IsWindowFocused();
		auto tex_size_vec = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };

		switch (mode_)
		{
			case wand_grabcut_mode::rectangle:
			{
				handle_rect_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
			}
			break;
			case wand_grabcut_mode::mask:
			{
				auto shape_data = data();
				if (shape_data != nullptr and is_focused)
				{
					bool is_definite = ImGui::GetIO().KeyShift;
					uint8_t color;
					if (is_eraser())
					{
						color = is_definite ? cv::GC_BGD : cv::GC_PR_BGD;
					}
					else
					{
						color = is_definite ? cv::GC_FGD : cv::GC_PR_FGD;
					}

					handle_drawing(shape_data, video_id, pos, size, tex_size, color);
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					{
						generate_mask(video_id, tex_size_vec);
					}
				}
				auto zoom_factor = size.x / tex_size.x;
				draw_brush_preview(ImGui::GetMousePos(), zoom_factor * brush_size());
			}
			break;
		}
	}

	void wand_grabcut_extension::render_properties()
	{
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_rect_selector, is_rect_mode()))
			{
				mode_ = wand_grabcut_mode::rectangle;
			}
			ui::tooltip("Rectangle");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_mask, is_mask_mode()))
			{
				mode_ = wand_grabcut_mode::mask;
			}
			ui::tooltip("Mask");
			ImGui::PopStyleVar();
		}
		if (is_mask_mode())
		{
			brush_tool::render_properties();
		}
		wand_tool_extension::render_properties();
	}

	void wand_grabcut_extension::on_deactivate()
	{
		reset();
	}

	void wand_grabcut_extension::on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size)
	{
		generate_mask(video_id, tex_size);
	}
}
