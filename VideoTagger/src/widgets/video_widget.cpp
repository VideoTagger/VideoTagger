#include "video_widget.hpp"

#include <string>
#include <imgui.h>
#include <utils/timestamp.hpp>

namespace vt::widgets
{
	void draw_video_widget(video& video, uint32_t id)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (video.is_open())
		{
			ImGui::SetNextWindowSize({ static_cast<float>(video.width()), static_cast<float>(video.height()) }, ImGuiCond_FirstUseEver);
		}
		float button_size = 25;
		std::string title = "Video##" + std::to_string(id);
		if (ImGui::Begin(title.c_str(), nullptr, flags))
		{
			ImGui::PushID(id);
			auto& imgui_style = ImGui::GetStyle();
			bool is_playing = video.is_playing();
			//window content here
			auto image_avail_size = ImGui::GetContentRegionMax();
			image_avail_size.y -= button_size + imgui_style.ItemSpacing.y;

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

				ImGui::Columns(3);
				{
					//static int current_offset = 0;
					//static clock_time_t time(30, 40, 20);
					//static time_widget_state state;

					std::chrono::nanoseconds video_ts = video.current_timestamp();
					timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video_ts) };
					timestamp duration{ std::chrono::duration_cast<std::chrono::seconds>(video.duration()) };

					auto avail_size = ImGui::GetContentRegionAvail();
					auto cursor_pos = ImGui::GetCursorPos();
					auto time_text_pos_x = avail_size.x / 2 - ImGui::CalcTextSize("00:00:00 | 00:00:00").x / 2;

					ImGui::SetCursorPosX(cursor_pos.x + time_text_pos_x);
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
					auto button_pos_x = avail_size.x / 2 - (button_size + imgui_style.ItemSpacing.x) * 4.f / 2;

					ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
					if (ImGui::Button("|<", { button_size, button_size }))
					{
						video.seek({});
					}
					ImGui::SameLine();
					if (ImGui::Button(is_playing ? "||" : ">", { button_size, button_size }))
					{
						video.set_playing(!is_playing);
					}
					ImGui::SameLine();
					if (ImGui::Button(">|", { button_size, button_size }))
					{
						video.seek(std::chrono::nanoseconds(video.duration()));
					}
					ImGui::SameLine();
					static bool loop = false;
					if (ImGui::Checkbox("##VideoPlayerLoop", &loop))
					{
						video.set_looping(loop);
					}
				}

				ImGui::NextColumn();
				{

					auto avail_size = ImGui::GetContentRegionAvail();
					float speed_control_size_x = avail_size.x * 0.75f;
					auto cursor_pos_x = ImGui::GetCursorPosX();
					auto speed_control_pos_x = avail_size.x / 2 - speed_control_size_x / 2;

					float speed = video.speed();
					static constexpr float min_speed = 0.25f;
					static constexpr float max_speed = 6.0f;

					ImGui::SetCursorPosX(cursor_pos_x + speed_control_pos_x);
					ImGui::SetNextItemWidth(speed_control_size_x);
					if (ImGui::DragFloat("##VideoPlayerSpeed", &speed, 0.1f, min_speed, max_speed, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						video.set_speed(speed);
					}
				}

				ImGui::Columns();
			}
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}
