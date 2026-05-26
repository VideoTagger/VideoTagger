#include "wand_sam2_extension.hpp"
#include <ui/widgets/common.hpp>
#include <ui/icons.hpp>
#include <core/app_context.hpp>
#include <models/sam2/sam2_model.hpp>
#include <utils/onnx.hpp>
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

	void wand_sam2_extension::generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size)
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

				ctx_.tasks.run([this, &vid_data, start = rect_data->start.max({ 0, 0 }), end = rect_data->end.min(tex_size)]()
				{
					set_busy(true);

					auto& stream = vid_data.video;
					image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
					if (stream.update_from_current_frame(img))
					{
						//TODO: This should only be created once
						auto env = utils::onnx_create_env();
						try
						{
							//TODO: This should not be hardcoded, and encoder/decoder should be a part of sam2_model class
							sam2_image_encoder encoder{ env, ctx_.models_dir_filepath / "sam2_hiera_large.encoder.onnx" };
							sam2_image_decoder decoder{ env, ctx_.models_dir_filepath / "sam2_hiera_large.decoder.onnx", encoder.input_size() };

							auto res = encoder.encode(img);

							auto dec_res = decoder.decode(res);
							cv::Mat result_mask = dec_res.masks[0];
								
							auto mask_data = data();
							if (mask_data == nullptr)
							{
								debug::error("Mask data is null, while generating sam2 mask");
							}
							else
							{
								mask_data->mask.allocate(result_mask.cols, result_mask.rows);
								auto cv_mask_data = image_to_cvmat(mask_data->mask);
								result_mask.copyTo(cv_mask_data);
								ctx_.tasks.run_on_main([this, result_mask]()
								{
									cv::imshow("SAM2 Mask", result_mask);
								});
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
		switch (mode_)
		{
			case wand_sam2_mode::rectangle:
			{
				handle_rect_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
				break;
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
		generate_mask(video_id, tex_size);
	}

}
