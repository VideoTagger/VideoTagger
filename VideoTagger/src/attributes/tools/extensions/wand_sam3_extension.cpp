#include "pch.hpp"
#include "wand_sam3_extension.hpp"
#include <ui/widgets/common.hpp>
#include <ui/icons.hpp>
#include <core/app_context.hpp>
#include <models/sam3/sam3_model.hpp>
#include <models/model_load_guard.hpp>
#include <opencv2/highgui.hpp>
#include <system/messagebox.hpp>

namespace vt::ui
{
	wand_sam3_extension::wand_sam3_extension(const std::string& name) : impl::wand_tool_extension{ name }, points_tool{ true }, mode_{ wand_sam3_mode::rectangle }, is_fg_point_{ true }, is_being_downloaded_{} {}

	bool wand_sam3_extension::is_rect_mode() const
	{
		return mode_ == wand_sam3_mode::rectangle;
	}

	bool wand_sam3_extension::is_points_mode() const
	{
		return mode_ == wand_sam3_mode::points;
	}

	bool wand_sam3_extension::is_mask_mode() const
	{
		return mode_ == wand_sam3_mode::mask;
	}

	void wand_sam3_extension::generate_mask(video_id_t video_id, const utils::vec2<int>& tex_size, const std::optional<utils::vec4<float>>& rect)
	{
		auto vid_it = ctx_.displayed_videos.find(video_id);
		if (vid_it == ctx_.displayed_videos.end()) return;
		auto& vid_data = *vid_it;

		switch (mode_)
		{
			case wand_sam3_mode::rectangle:
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
							auto sam = ctx_.model_registry.get_model<sam3_model>();
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

							sam3_decoder_prompt prompt;
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
								//	cv::imshow("SAM Mask", result_mask);
								//});
							}
						}
						catch (const std::exception& e)
						{
							debug::error("SAM encoder/decoder error: {}", e.what());
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
			case wand_sam3_mode::points:
			{
				auto& foreground_points = points_tool::fg_points();
				auto& background_points = points_tool::bg_points();

				if (foreground_points.points.empty() and background_points.points.empty())
				{
					auto mask_data = data();
					if (mask_data != nullptr)
					{
						mask_data->mask.clear();
						mask_data->recalculate_bounding_box();
					}
					return;
				}

				ctx_.tasks.run([this, &vid_data, &foreground_points, &background_points]()
				{
					set_busy(true);

					auto& stream = vid_data.video;
					image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
					if (stream.update_from_current_frame(img))
					{
						try
						{
							auto sam = ctx_.model_registry.get_model<sam3_model>();
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

							sam3_decoder_prompt prompt;
							for (auto& point : foreground_points.points)
							{
								sam3_decoder_prompt_point prompt_point;
								prompt_point.label = sam3_label::foreground;
								prompt_point.point = { static_cast<float>(point.x()), static_cast<float>(point.y()) };
								prompt.points.push_back(prompt_point);
							}
							for (auto& point : background_points.points)
							{
								sam3_decoder_prompt_point prompt_point;
								prompt_point.label = sam3_label::background;
								prompt_point.point = { static_cast<float>(point.x()), static_cast<float>(point.y()) };
								prompt.points.push_back(prompt_point);
							}
							auto dec_res = decoder->decode(res, prompt);
							if (dec_res.masks.empty())
							{
								set_busy(false);
								return;
							}

							if (dec_res.masks.empty())
							{
								set_busy(false);
								debug::error("SAM 3 decoder returned no masks");
								return;
							}

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
								//	cv::imshow("SAM Mask", result_mask);
								//});
							}
						}
						catch (const std::exception& e)
						{
							debug::error("SAM3 encoder/decoder error: {}", e.what());
						}
					}
					set_busy(false);
				})
					/*.then(ctx_.tasks.on_main(), [this]()
					{
						points_tool::reset();
					})*/;
			}
			break;
		}
	}

	void wand_sam3_extension::reset()
	{
		mode_ = wand_sam3_mode::rectangle;
		rect_select_tool::reset();
		points_tool::reset();
		is_fg_point_ = true;
		set_data(nullptr);
	}

	uint32_t wand_sam3_extension::property_column_count() const
	{
		auto col_count = 1;
		if (is_mask_mode())
		{
			col_count += brush_tool::property_column_count();
		}
		return col_count;
	}

	void wand_sam3_extension::render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		if (is_busy()) return;

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_focused = ImGui::IsWindowFocused();
		auto tex_size_vec = utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };

		switch (mode_)
		{
			case wand_sam3_mode::rectangle:
			{
				handle_rect_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
			}
			break;
			case wand_sam3_mode::points:
			{
				points_tool::handle_point_selection(video_id, ImRect(pos, pos + size), { static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) });
			}
			break;
			case wand_sam3_mode::mask:
			{
				auto shape_data = data();
				if (shape_data != nullptr and is_focused)
				{
					handle_drawing(shape_data, video_id, pos, size, tex_size, is_eraser() ? 0 : 255);
				}
				auto zoom_factor = size.x / tex_size.x;
				draw_brush_preview(ImGui::GetMousePos(), zoom_factor * brush_size());
			}
			break;
		}
	}

	void wand_sam3_extension::render_properties()
	{
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_toggle_button(icons::tool_rect_selector, is_rect_mode()))
			{
				mode_ = wand_sam3_mode::rectangle;
			}
			ui::tooltip("Rectangle");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_points, is_points_mode()))
			{
				mode_ = wand_sam3_mode::points;
			}
			ui::tooltip("Points");
			ImGui::SameLine();
			if (ui::icon_toggle_button(icons::tool_mask, is_mask_mode()))
			{
				mode_ = wand_sam3_mode::mask;
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

	void wand_sam3_extension::on_deactivate()
	{
		reset();
	}

	void wand_sam3_extension::on_done()
	{
		rect_select_tool::reset();
		points_tool::reset();
		set_data(nullptr);
	}

	void wand_sam3_extension::on_finish_selection(video_id_t video_id, const rectangle_shape& rect, const utils::vec2<int>& tex_size)
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

	void wand_sam3_extension::on_finish_point_selection(video_id_t video_id, const utils::vec2<int>& tex_size)
	{
		generate_mask(video_id, tex_size);
	}

	void wand_sam3_extension::prepare_for_use()
	{
		auto sam = ctx_.model_registry.get_model<sam3_model>();
		if (sam != nullptr and !sam->is_downloaded() and !is_being_downloaded_)
		{
			messagebox_data data{};
			data.icon = messagebox_icon::info;
			data.title = "Model download required";
			data.buttons =
			{
				{ 0, ctx_.lang->get("generic.yes")},
				{ 1, ctx_.lang->get("cancel") },
			};
			data.message = "This action requires downloading SAM 3 model files (~X MB/GB).\nWould you like to proceed with the download?";
			data.cancel_button_id = 1;
			data.default_button_id = 0;
			data.callback = [this, sam](int button_id)
			{
				switch (button_id)
				{
					case 0:
					{
						is_being_downloaded_ = true;
						sam->download(false, [this]()
						{
							is_being_downloaded_ = false;
						});
					}
					break;
				}
			};
			messagebox::show(data);
		}
	}

	bool wand_sam3_extension::is_ready()
	{
		auto sam = ctx_.model_registry.get_model<sam3_model>();
		return sam != nullptr and sam->is_downloaded();
	}

}
