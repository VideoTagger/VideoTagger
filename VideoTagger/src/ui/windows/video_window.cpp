#include "video_window.hpp"
#include <widgets/slider.hpp>
#include <ui/widgets/common.hpp>
#include <utils/timestamp.hpp>
#include <ui/icons.hpp>
#include <core/app_context.hpp>

namespace vt::ui::windows
{
	video_window::video_window(uint64_t id) : window
	{
		"video-window-" + std::to_string(id), "video-window-" + std::to_string(id), "Video",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings, false
	}, video_{}, texture_{}, is_active_{}, is_interactive_{ true }, id_{ id }, video_id_{}, scale_{ 1.0f }, offset_{}
	{
		set_persistent(false);
		set_icon(icons::video);
	}

	video_window& video_window::with_overlay(const std::function<void(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)>& overlay)
	{
		overlays_.push_back(overlay);
		return *this;
	}

	void video_window::clear_overlays()
	{
		overlays_.clear();
	}

	void video_window::render_overlays(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
	{
		for (const auto& overlay : overlays_)
		{
			overlay(video_id, pos, size, tex_size);
		}
	}

	void video_window::set_video(video_stream& video, video_id_t video_id)
	{
		video_ = &video;
		video_id_ = video_id;
	}

	void video_window::set_texture(gl_texture& texture)
	{
		texture_ = &texture;
	}

	void video_window::set_active(bool value)
	{
		is_active_ = value;
	}

	void video_window::on_zoom(float zoom_factor, ImVec2 video_screen_pos, ImVec2 zoom_center)
	{
		auto new_scale = std::clamp(scale_ * zoom_factor, 0.1f, 10.0f);
		offset_ += (zoom_center - video_screen_pos) * (1.0f - new_scale / scale_);
		scale_ = new_scale;
	}

	void video_window::pre_style()
	{
		if (video_->is_open())
		{
			ImGui::SetNextWindowSize({ static_cast<float>(video_->width()), static_cast<float>(video_->height()) }, ImGuiCond_FirstUseEver);
		}

		const auto& style = ImGui::GetStyle();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
	}

	void video_window::post_style()
	{
		ImGui::PopStyleVar();
	}

	void video_window::on_render()
	{
		if (video_ == nullptr or texture_ == nullptr) return;

		auto& io = ImGui::GetIO();
		float button_size = 25 * io.FontGlobalScale;

		bool show_controls = true;
		auto video_window = ImGui::GetCurrentWindow();

		//A bit of a hack to check if video widget is docked into video player
		//TODO: There might be a better way to do this
		if (video_window->ParentWindow != nullptr)
		{
			std::string name = video_window->ParentWindow->Name;
			show_controls = (name.find("Video Player") == std::string::npos);
		}

		auto str_id = id();
		ImGui::PushID(str_id.c_str());
		auto& imgui_style = ImGui::GetStyle();
		bool is_playing = false; //video.is_playing();
		auto image_avail_size = ImGui::GetContentRegionMax();

		//TODO: a video probably shouldn't have its own controls since they could break synchronization 
		show_controls = false;
		if (show_controls)
		{
			image_avail_size.y -= button_size + 2 * imgui_style.ItemSpacing.y + ImGui::GetTextLineHeightWithSpacing() * io.FontGlobalScale;
		}

		if (texture_->id() != 0)
		{
			int video_width = video_->width();
			int video_height = video_->height();

			float scaled_width = video_width * image_avail_size.y / video_height;
			float scaled_height = image_avail_size.x * video_height / video_width;

			ImVec2 image_size = image_avail_size;
			if (scaled_width < image_avail_size.x)
			{
				image_size.x = scaled_width;
			}
			else if (scaled_height < image_avail_size.y)
			{
				image_size.y = scaled_height;
			}

			ImVec2 video_cursor_pos = { (image_avail_size.x - image_size.x) / 2 + offset_.x, (image_avail_size.y - image_size.y) / 2 + offset_.y };

			image_size.x *= scale_;
			image_size.y *= scale_;

			ImGui::SetCursorPos({ video_cursor_pos });
			const ImVec2 video_screen_pos = ImGui::GetCursorScreenPos();

			ImRect img_rect(video_cursor_pos, { video_cursor_pos.x + image_size.x, video_cursor_pos.y + image_size.y });
			ImGui::Dummy(img_rect.GetSize());
			const auto& io = ImGui::GetIO();

			bool is_move_tool_active = ctx_.session.toolbar.is_tool_active("move");
			bool is_zoom_tool_active = ctx_.session.toolbar.is_tool_active("magnifier");
			if (is_interactive_)
			{
				bool is_vid_hovered = ImGui::IsItemHovered();
				bool is_win_hovered = ImGui::IsWindowHovered();

				if (is_move_tool_active)
				{
					const auto mouse_offset = io.MousePos - video_cursor_pos;
					
					if (is_win_hovered)
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							last_mouse_pos_ = mouse_offset;
						}

						if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
						{
							ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
							offset_ += (mouse_offset - last_mouse_pos_) * (1.0f / scale_);
						}
					}

					if (is_vid_hovered and io.MouseWheel != 0)
					{
						auto zoom_factor = 1.0f + (io.MouseWheel > 0 ? 0.1f : -0.1f);
						on_zoom(zoom_factor, video_screen_pos, io.MousePos);
					}
				}
				if (is_zoom_tool_active and is_vid_hovered)
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						on_zoom(1.1f, video_screen_pos, io.MousePos);
					}
					else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						on_zoom(0.9f, video_screen_pos, io.MousePos);
					}
				}
			}

			ImGui::SetCursorPos({ video_cursor_pos });

			ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)texture_->id()), image_size);

			if (!is_active_)
			{
				//TODO: tweak colors
				auto border_color = ImGui::ColorConvertFloat4ToU32({ 0.2f, 0.2f, 0.2f, 1.0f });
				auto overlay_color = ImGui::ColorConvertFloat4ToU32({ 0.3f, 0.3f, 0.3f, 0.8f });
				float border_thickness = 2.0f;

				ImVec2 top_left = { video_screen_pos.x, video_screen_pos.y };
				ImVec2 top_right = { video_screen_pos.x + image_size.x, video_screen_pos.y };
				ImVec2 bottom_left = { video_screen_pos.x, video_screen_pos.y + image_size.y };
				ImVec2 bottom_right = { video_screen_pos.x + image_size.x, video_screen_pos.y + image_size.y };

				auto draw_list = ImGui::GetWindowDrawList();
				draw_list->AddRectFilled(top_left, bottom_right, overlay_color);
				draw_list->AddRect(top_left, bottom_right, border_color, 0, 0, border_thickness);
				draw_list->AddLine(top_left, bottom_right, border_color, border_thickness);
				draw_list->AddLine(top_right, bottom_left, border_color, border_thickness);
			}
			else
			{
				render_overlays(video_id_, video_screen_pos, image_size, { (float)texture_->width(), (float)texture_->height() });
			}

			const auto& current_frame = video_->current_frame();
			auto video_ts = current_frame.has_value() ? current_frame->timestamp() : std::chrono::nanoseconds{};
			auto duration_ts = video_->duration();
			timestamp current_time{ std::chrono::duration_cast<std::chrono::milliseconds>(video_ts) };
			timestamp duration{ std::chrono::duration_cast<std::chrono::milliseconds>(duration_ts) };
			decltype(video_ts) min_ts{};

			if (show_controls)
			{
				auto text_height = ImGui::GetTextLineHeightWithSpacing();
				if (widgets::slider_scalar("##VideoProgressBar", ImGuiDataType_U64, ImVec2{ ImGui::GetContentRegionAvail().x, text_height }, text_height / 5.f, &video_ts, &min_ts, &duration_ts, "", ImGuiSliderFlags_AlwaysClamp))
				{
					video_->seek(video_ts);
				}
				ImGui::Columns(3);
				{
					//static int current_offset = 0;
					//static clock_time_t time(30, 40, 20);
					//static time_widget_state state;					

					auto avail_size = ImGui::GetContentRegionAvail();
					auto text_size = ImGui::CalcTextSize("00:00:00 | 00:00:00");

					ImGui::SetCursorPos({ avail_size.x - text_size.x, ImGui::GetCursorPosY() + text_size.y / 4 });
					ImGui::Text("%02ld:%02ld:%02ld | %02ld:%02ld:%02ld",
						current_time.hours(), current_time.minutes(), current_time.seconds(),
						duration.hours(), duration.minutes(), duration.seconds()
					);

					//static time_widget_state state;

					//if (input_time(state, current_time))
					//{
					//	video.seek(std::chrono::duration_cast<timestamp_t>(current_time.seconds_total));
					//}
					//int64_t ts = video.current_timestamp().count();
					//if (ImGui::InputScalar("time", ImGuiDataType_S64, (void*)&ts, nullptr, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue))
					//{
					//	video.seek(timestamp_t(ts));
					//}
				}

				ImGui::NextColumn();
				{
					auto avail_size = ImGui::GetContentRegionAvail();
					auto cursor_pos = ImGui::GetCursorPos();
					auto button_pos_x = avail_size.x / 2 - (button_size + imgui_style.ItemSpacing.x) * 5.f / 2;

					ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
					if (ui::icon_button(icons::skip_prev, { button_size, button_size })) {}
					ImGui::SameLine();
					if (ui::icon_button(icons::fast_back, { button_size, button_size }))
					{
						video_->seek({});
					}
					ImGui::SameLine();
					if (ui::icon_button(is_playing ? icons::pause : icons::play, { button_size, button_size }))
					{
						//video.set_playing(!is_playing);
					}
					ImGui::SameLine();
					if (ui::icon_button(icons::fast_fwd, { button_size, button_size }))
					{
						video_->seek(video_->duration());
					}
					ImGui::SameLine();
					if (ui::icon_button(icons::skip_next, { button_size, button_size })) {}
				}

				ImGui::NextColumn();
				{
					//bool loop = video.is_looping();
					//if (icon_toggle_button(icons::repeat, loop, { button_size, button_size }))
					//{
					//	video.set_looping(!loop);
					//}
					//ImGui::SameLine();

					auto avail_size = ImGui::GetContentRegionAvail();
					float speed_control_size_x = avail_size.x * 0.5f;

					//float speed = video.speed();
					static constexpr float min_speed = 0.25f;
					static constexpr float max_speed = 8.0f;

					//ImGui::SetNextItemWidth(speed_control_size_x);
					//if (ImGui::DragFloat("##VideoPlayerSpeed", &speed, 0.1f, min_speed, max_speed, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
					//{
					//	video.set_speed(speed);
					//}
					//if (ImGui::BeginPopupContextItem("##VideoPlayerSpeedCtx"))
					//{
					//	if (ImGui::MenuItem("Reset"))
					//	{
					//		speed = 1.0f;
					//		video.set_speed(speed);
					//	}
					//	ImGui::EndPopup();
					//}
				}

				ImGui::Columns();
			}
		}
		ImGui::PopID();
	}
}
