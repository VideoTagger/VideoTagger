#include "wand_watershed_extension.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>
#include <image/image_opencv.hpp>
#include <opencv2/highgui.hpp>

namespace vt::ui
{
	wand_watershed_extension::wand_watershed_extension(const std::string& name) : impl::wand_tool_extension{ name }, vt::impl::points_tool{ false }, mode_{ wand_watershed_mode::points }, is_fg_brush_{ true } {}

	bool wand_watershed_extension::is_points_mode() const
	{
		return mode_ == wand_watershed_mode::points;
	}

	bool wand_watershed_extension::is_mask_mode() const
	{
		return mode_ == wand_watershed_mode::mask;
	}

	void wand_watershed_extension::generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size)
	{
		auto vid_it = ctx_.displayed_videos.find(video_id);
		if (vid_it == ctx_.displayed_videos.end()) return;
		auto& vid_data = *vid_it;

		switch (mode_)
		{
			case wand_watershed_mode::points:
			{
				ctx_.tasks.run([this, &vid_data]()
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

						cv::Mat gray;
						cv::cvtColor(cv_img, gray, cv::COLOR_BGR2GRAY);
						cv::Mat threshold;
						cv::threshold(gray, threshold, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
						const int kernel_size = 3;
						const int morph_iterations = 2;
						cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_size, kernel_size));
						cv::Mat opening;
						cv::morphologyEx(threshold, opening, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), morph_iterations);
						
						//sure background
						const int dilate_iterations = 3;
						cv::Mat sure_bg;
						cv::dilate(opening, sure_bg, kernel, cv::Point(-1, -1), dilate_iterations);
						
						//sure foreground
						cv::Mat dist_transform;
						cv::distanceTransform(opening, dist_transform, cv::DIST_L2, 5);
						double dist_transform_max = 0.0;
						cv::minMaxLoc(dist_transform, nullptr, &dist_transform_max);
						cv::Mat sure_fg;
						cv::threshold(dist_transform, sure_fg, 0.7 * dist_transform_max, 255, cv::THRESH_BINARY);
						sure_fg.convertTo(sure_fg, CV_8U);

						cv::Mat unknown;
						cv::subtract(sure_bg, sure_fg, unknown);

						cv::Mat markers;
						cv::connectedComponents( sure_fg, markers, 8, CV_32S);
						
						//removes unknown areas
						markers.setTo(0, unknown == 255);

						//next label for new points
						double max_marker_value = 0.0;
						cv::minMaxLoc(markers, nullptr, &max_marker_value);
						int next_label = static_cast<int>(max_marker_value) + 1;

						auto& points_data = points_tool::fg_points();
						std::vector<int> selected_labels;
						for (const auto& point : points_data.points)
						{
							const int x = point.x();
							const int y = point.y();
							if (x < 0 or y < 0 or x >= markers.cols or y >= markers.rows) continue;
							const int marker_label = next_label++;
							markers.at<int>(y, x) = marker_label;
							selected_labels.push_back(marker_label);
						}
						if (selected_labels.empty())
						{
							debug::error("Watershed requires at least one valid point");
							set_busy(false);
							return;
						}

						cv::watershed(cv_img, markers);
						cv::Mat result_mask = cv::Mat::zeros(markers.size(), CV_8UC1);
						for (int y = 0; y < markers.rows; ++y)
						{
							const int* marker_row = markers.ptr<int>(y);
							auto* mask_row = result_mask.ptr<uint8_t>(y);
							for (int x = 0; x < markers.cols; ++x)
							{
								const int marker = marker_row[x];
								if (std::find(selected_labels.begin(), selected_labels.end(), marker) != selected_labels.end())
								{
									//positive: foreground, negative: background, 0: unknown
									mask_row[x] = 255;
								}
							}
						}

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
					}
				});
			}
			break;
		}
		//reset();
	}

	void wand_watershed_extension::reset()
	{
		mode_ = wand_watershed_mode::points;
		points_tool::reset();
		set_data(nullptr);
	}

	void wand_watershed_extension::on_done()
	{
		points_tool::reset();
	}

	uint32_t wand_watershed_extension::property_column_count() const
	{
		auto col_count = 1;
		if (is_mask_mode())
		{
			col_count += brush_tool::property_column_count();
		}
		return col_count;
	}

	void wand_watershed_extension::on_finish_selection(video_id_t video_id, const utils::vec2<int>& tex_size)
	{
		generate_mask(video_id, tex_size);
	}

	void wand_watershed_extension::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
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
			case wand_watershed_mode::points:
			{
				points_tool::handle_point_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
			}
			break;
		}
	}

	void wand_watershed_extension::render_properties()
	{
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_points, is_points_mode()))
			{
				mode_ = wand_watershed_mode::points;
			}
			ui::tooltip("Points");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_mask, is_mask_mode()))
			{
				mode_ = wand_watershed_mode::mask;
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

	void wand_watershed_extension::on_finish_point_selection(video_id_t video_id, const utils::vec2<int>& tex_size)
	{
		on_finish_selection(video_id, tex_size);
	}
}
