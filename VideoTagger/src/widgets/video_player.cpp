#include "video_player.hpp"
#include <imgui.h>
#include <string>
#include <chrono>
#include <utils/timestamp.hpp>
#include "slider.hpp"
#include "buttons.hpp"
#include "icons.hpp"

namespace vt::widgets
{
	video_player::video_player() : is_playing{}, is_looping{}
    {

    }

	void video_player::render()
	{
		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		float button_size = 25 * io.FontGlobalScale;
		std::string title = "Video Player";

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		bool is_open = ImGui::Begin(title.c_str(), nullptr, flags);

		if (is_open)
		{
			auto& imgui_style = ImGui::GetStyle();
			auto image_avail_size = ImGui::GetContentRegionAvail();
			auto text_height = ImGui::GetTextLineHeightWithSpacing();
			image_avail_size.y -= button_size + 2 * imgui_style.ItemSpacing.y + text_height * io.FontGlobalScale + 2 * style.FramePadding.y;

			if (ImGui::BeginChild("##VideoPlayerFrame", image_avail_size))
			{
				auto flags = ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode;
				ImGui::DockSpace(ImGui::GetID("##VideoPlayerFrameDockspace"), ImGui::GetContentRegionAvail(), flags);
			}
			ImGui::EndChild();

			//TODO: Use actual values
			auto video_ts = std::chrono::nanoseconds{};
			auto duration_ts = std::chrono::nanoseconds{ 1500 };
			timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video_ts) };
			timestamp duration{ std::chrono::duration_cast<std::chrono::seconds>(duration_ts) };
			decltype(video_ts) min_ts{};

			ImGui::BeginGroup();
			if (slider_scalar("##VideoProgressBar", ImGuiDataType_U64, ImVec2{ ImGui::GetContentRegionAvail().x, text_height }, text_height / 5.f, &video_ts, &min_ts, &duration_ts, "", ImGuiSliderFlags_AlwaysClamp))
			{
				//TODO: Seek videos
				//video.seek(video_ts);
			}
			ImGui::Columns(3);
			{
				auto avail_size = ImGui::GetContentRegionAvail();
				auto text_size = ImGui::CalcTextSize("00:00:00 | 00:00:00");

				ImGui::SetCursorPos({ avail_size.x - text_size.x, ImGui::GetCursorPosY() + text_size.y / 4 });
				ImGui::Text("%02d:%02d:%02d | %02d:%02d:%02d",
					current_time.hours(), current_time.minutes(), current_time.seconds(),
					duration.hours(), duration.minutes(), duration.seconds()
				);
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
					//TODO: Seek videos
					//video.seek({});
				}
				ImGui::SameLine();
				if (icon_button(is_playing ? icons::pause : icons::play, { button_size, button_size }))
				{
					//TODO: Pause/unpause videos
					//video.set_playing(!is_playing);
				}
				ImGui::SameLine();
				if (icon_button(icons::fast_fwd, { button_size, button_size }))
				{
					//TODO: Seek videos
					//video.seek(std::chrono::nanoseconds(video.duration()));
				}
				ImGui::SameLine();
				if (icon_button(icons::skip_next, { button_size, button_size })) {}
			}

			ImGui::NextColumn();
			{
				//TODO: Implement looping
				bool loop = false /*video.is_looping()*/;
				if (icon_toggle_button(icons::repeat, loop, { button_size, button_size }))
				{
					//video.set_looping(!loop);
				}
				ImGui::SameLine();

				auto avail_size = ImGui::GetContentRegionAvail();
				float speed_control_size_x = avail_size.x * 0.5f;

				//TODO: Implement this
				float speed = 1.0f/*video.speed()*/;
				static constexpr float min_speed = 0.25f;
				static constexpr float max_speed = 8.0f;

				ImGui::SetNextItemWidth(speed_control_size_x);
				if (ImGui::DragFloat("##VideoPlayerSpeed", &speed, 0.1f, min_speed, max_speed, "%.2fx", ImGuiSliderFlags_AlwaysClamp))
				{
					//video.set_speed(speed);
				}
				if (ImGui::BeginPopupContextItem("##VideoPlayerSpeedCtx"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						speed = 1.0f;
						//video.set_speed(speed);
					}
					ImGui::EndPopup();
				}
			}
			ImGui::EndGroup();
			ImGui::Columns();
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}
}
