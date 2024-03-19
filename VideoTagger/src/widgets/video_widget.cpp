#include "video_widget.hpp"

#include <string>
#include <imgui.h>
#include <imgui_internal.h>
#include <utils/timestamp.hpp>
#include "slider.hpp"
#include "controls.hpp"
#include "icons.hpp"
#include <core/debug.hpp>

#include <core/app.hpp>

namespace vt::widgets
{
	void draw_video_widget(video& video, bool& is_open, uint64_t id)
	{
		auto& io = ImGui::GetIO();
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (video.is_open())
		{
			ImGui::SetNextWindowSize({ static_cast<float>(video.width()), static_cast<float>(video.height()) }, ImGuiCond_FirstUseEver);
		}
		float button_size = 25 * io.FontGlobalScale;
		std::string str_id = std::to_string(id);
		std::string title = "Video##" + str_id;
		if (ImGui::Begin(title.c_str(), &is_open, flags))
		{
			bool show_controls = true;
			auto video_window = ImGui::GetCurrentWindow();
			
			//A bit of a hack to check if video widget is docked into video player
			//TODO: There might be a better way to do this
			if (video_window->ParentWindow != nullptr)
			{
				std::string name = video_window->ParentWindow->Name;
				show_controls = (name.find("Video Player") == std::string::npos);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
			ImGui::PushID(str_id.c_str());
			auto& imgui_style = ImGui::GetStyle();
			bool is_playing = video.is_playing();
			auto image_avail_size = ImGui::GetContentRegionMax();

			//TODO: a video probably shouldn't have its own controls since they could break synchronization 
			show_controls = false;
			if (show_controls)
			{
				image_avail_size.y -= button_size + 2 * imgui_style.ItemSpacing.y + ImGui::GetTextLineHeightWithSpacing() * io.FontGlobalScale;
			}

			SDL_Texture* texture = video.get_frame();
			if (texture != nullptr)
			{
				int video_width = video.width();
				int video_height = video.height();

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

				ImGui::SetCursorPos({ (image_avail_size.x - image_size.x) / 2, (image_avail_size.y - image_size.y) / 2 });
				ImGui::Image((ImTextureID)texture, image_size);
				ImGui::PopStyleVar();

				auto video_ts = video.current_timestamp();
				auto duration_ts = video.duration();
				timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video_ts) };
				timestamp duration{ std::chrono::duration_cast<std::chrono::seconds>(duration_ts) };
				decltype(video_ts) min_ts{};

				if (show_controls)
				{
					auto text_height = ImGui::GetTextLineHeightWithSpacing();
					if (slider_scalar("##VideoProgressBar", ImGuiDataType_U64, ImVec2{ ImGui::GetContentRegionAvail().x, text_height }, text_height / 5.f, &video_ts, &min_ts, &duration_ts, "", ImGuiSliderFlags_AlwaysClamp))
					{
						video.seek(video_ts);
					}
					ImGui::Columns(3);
					{
						//static int current_offset = 0;
						//static clock_time_t time(30, 40, 20);
						//static time_widget_state state;					

						auto avail_size = ImGui::GetContentRegionAvail();
						auto text_size = ImGui::CalcTextSize("00:00:00 | 00:00:00");

						ImGui::SetCursorPos({ avail_size.x - text_size.x, ImGui::GetCursorPosY() + text_size.y / 4 });
						ImGui::Text("%02d:%02d:%02d | %02d:%02d:%02d",
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
						if (icon_button(icons::skip_prev, { button_size, button_size })) {}
						ImGui::SameLine();
						if (icon_button(icons::fast_back, { button_size, button_size }))
						{
							video.seek({});
						}
						ImGui::SameLine();
						if (icon_button(is_playing ? icons::pause : icons::play, { button_size, button_size }))
						{
							video.set_playing(!is_playing);
						}
						ImGui::SameLine();
						if (icon_button(icons::fast_fwd, { button_size, button_size }))
						{
							video.seek(std::chrono::nanoseconds(video.duration()));
						}
						ImGui::SameLine();
						if (icon_button(icons::skip_next, { button_size, button_size })) {}
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

						ImGui::SetNextItemWidth(speed_control_size_x);
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
		ImGui::End();
	}
}
