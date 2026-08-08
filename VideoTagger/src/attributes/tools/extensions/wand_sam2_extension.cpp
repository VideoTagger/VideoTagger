#include "wand_sam2_extension.hpp"
#include <ui/widgets/common.hpp>
#include <ui/icons.hpp>
#include <core/app_context.hpp>
#include <models/sam2/sam2_model.hpp>
#include <models/model_load_guard.hpp>
#include <opencv2/highgui.hpp>

namespace vt::ui
{
	wand_sam2_extension::wand_sam2_extension(const std::string& name) : impl::wand_tool_extension{ name }, mode_{ wand_sam2_mode::rectangle }
	{

	}

	bool wand_sam2_extension::is_rect_mode() const
	{
		return mode_ == wand_sam2_mode::rectangle;
	}

	bool wand_sam2_extension::is_points_mode() const
	{
		return mode_ == wand_sam2_mode::points;
	}

	bool wand_sam2_extension::is_mask_mode() const
	{
		return mode_ == wand_sam2_mode::mask;
	}

	void wand_sam2_extension::generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size, const std::optional<utils::vec4<float>>& rect)
	{
		auto vid_it = ctx_.displayed_videos.find(video_id);
		if (vid_it == ctx_.displayed_videos.end()) return;
		auto& vid_data = *vid_it;

		switch (mode_)
		{
			case wand_sam2_mode::rectangle:
			{
				auto& rect_data = rect_select_data();
				if (rect_data == nullptr or rect_data->start == rect_data->end) return;

				ctx_.tasks.run([this, rect = rect, &vid_data, start = rect_data->start.max({ 0, 0 }), end = rect_data->end.min(tex_size)]()
				{
					set_busy(true);

					auto& stream = vid_data.video;
					image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
					if (stream.update_from_current_frame(img))
					{
						try
						{
							auto sam = ctx_.model_registry.get_model<sam2_model>();
							if (sam == nullptr or !sam->load_if_needed())
							{
								throw std::runtime_error("Failed to load model");
							}
							auto load_guard = model_load_guard{ sam };

							auto encoder = sam->encoder();
							if (encoder == nullptr) throw std::runtime_error("Encoder is null");
							auto decoder = sam->decoder();
							if (decoder == nullptr) throw std::runtime_error("Decoder is null");

							auto res = encoder->encode(img);

							sam2_decoder_prompt prompt;
							prompt.rect = rect;
							auto dec_res = decoder->decode(res, prompt);

							cv::Mat result_mask = dec_res.masks[0];
							load_guard.release();
								
							auto mask_data = data();
							if (mask_data == nullptr)
							{
								debug::error("Mask data is null, while generating mask");
							}
							else
							{
								mask_data->mask.allocate(result_mask.cols, result_mask.rows);
								auto cv_mask_data = image_to_cvmat(mask_data->mask);
								result_mask.copyTo(cv_mask_data);
								//ctx_.tasks.run_on_main([this, result_mask]()
								//{
								//	cv::imshow("SAM2 Mask", result_mask);
								//});
							}
						}
						catch (const std::exception& e)
						{
							debug::error("SAM2 encoder/decoder error: {}", e.what());
						}
					}
					set_busy(false);
				})
				.then(ctx_.tasks.on_main(), [this]()
				{
					rect_select_tool::reset();
				});
			}
			break;
		}
	}

	void wand_sam2_extension::reset()
	{
		mode_ = wand_sam2_mode::rectangle;
		rect_select_tool::reset();
	}

	uint32_t wand_sam2_extension::property_column_count() const
	{
		auto col_count = 1;
		if (is_mask_mode())
		{
			col_count += brush_tool::property_column_count();
		}
		return col_count;
	}

	void wand_sam2_extension::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		if (is_busy()) return;

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_focused = ImGui::IsWindowFocused();
		auto tex_size_vec = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };

		switch (mode_)
		{
			case wand_sam2_mode::rectangle:
			{
				handle_rect_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
				break;
			}
			case wand_sam2_mode::mask:
			{
				auto shape_data = data();
				if (shape_data != nullptr and is_focused)
				{
					handle_drawing(shape_data, video_id, pos, size, tex_size, is_eraser() ? 0 : 255);
				}
				auto zoom_factor = size.x / tex_size.x;
				draw_brush_preview(ImGui::GetMousePos(), zoom_factor * brush_size());
			}
		}
	}

	void wand_sam2_extension::render_properties()
	{
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_rect_selector, is_rect_mode()))
			{
				mode_ = wand_sam2_mode::rectangle;
			}
			ui::tooltip("Rectangle");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_points, is_points_mode()))
			{
				mode_ = wand_sam2_mode::points;
			}
			ui::tooltip("Points");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_mask, is_mask_mode()))
			{
				mode_ = wand_sam2_mode::mask;
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

	void wand_sam2_extension::on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size)
	{
		bool has_rect = rect.start != rect.end;
		std::optional<utils::vec4<float>> vec_rect;
		if (has_rect)
		{
			auto start_x = std::clamp(std::min(rect.start.x(), rect.end.x()), 0, tex_size.x());
			auto start_y = std::clamp(std::min(rect.start.y(), rect.end.y()), 0, tex_size.y());
			auto end_x = std::clamp(std::max(rect.start.x(), rect.end.x()), 0, tex_size.x());
			auto end_y = std::clamp(std::max(rect.start.y(), rect.end.y()), 0, tex_size.y());
			
			vec_rect =
			{
				static_cast<float>(start_x),
				static_cast<float>(start_y),
				static_cast<float>(end_x),
				static_cast<float>(end_y)
			};
		}
		generate_mask(video_id, tex_size, vec_rect);
	}

}
