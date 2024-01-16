#include "widgets.hpp"

#include <string>
#include <vector>
#include <array>
#include <string_view>

#include <iostream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "video_timeline.hpp"
#include <utils/timestamp.hpp>

namespace vt::widgets
{
	struct time_widget_state
	{
		static constexpr std::string_view time_string_template = "00:00:00";
		static constexpr std::string_view time_string_format = "%02:%02d:%02d";
		static constexpr size_t buffer_size = time_string_template.size() + 1;
		
		int current_offset{};
		char buffer[buffer_size]{};
		bool active{};
	};

	void draw_video_widget(video& video)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (video.is_open())
		{
			ImGui::SetNextWindowSize({ float(video.width()), float(video.height()) }, ImGuiCond_FirstUseEver);
		}
		float button_size = 25;
		if (ImGui::Begin("Video", nullptr, flags))
		{
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

					auto video_ts = video.current_timestamp();
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

					static time_widget_state state;
					
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
						video.seek(std::chrono::nanoseconds(0));
					}
					ImGui::SameLine();
					if (ImGui::Button(is_playing ? "||" : ">", { button_size, button_size }))
					{
						video.set_playing(!is_playing);
					}
					ImGui::SameLine();
					if (ImGui::Button(">|", { button_size, button_size }))
					{
						video.seek(video.duration());
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
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
	
	void draw_timeline_widget_sample(tag_storage& tags, video& video)
	{
		static timeline_state test_timeline;
		test_timeline.tags = &tags;
		test_timeline.sync_tags();

		if (video.is_open())
		{
			test_timeline.time_max = timestamp(std::chrono::duration_cast<std::chrono::seconds>(video.duration()));
		}
		else
		{
			test_timeline.time_min = timestamp{};
		}

		static int selected_entry = -1;
		static int64_t first_frame = 0;
		static bool expanded = true;
		timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()) };

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin("Timeline"))
		{
			video_timeline(&test_timeline, &current_time, &selected_entry);
			
			if (current_time.seconds_total != std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()))
			{
				video.seek(current_time.seconds_total);
			}
			
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void draw_tag_manager_widget(tag_storage& tags)
	{
		if (ImGui::Begin("Tags test"))
		{
			static tag_storage::iterator selected = tags.end();
			if (widgets::tag_manager(tags, selected))
			{
				std::cout << "selected tag " << selected->name << "\n";
			}
		}
		ImGui::End();
	}
}
