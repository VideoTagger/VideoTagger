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
	
	void draw_timeline_widget_sample(timeline_state& state, video& video, tag_storage& tags, std::optional<selected_timestamp_data>& selected_timestamp, std::optional<moving_timestamp_data>& moving_timestamp, bool& dirty_flag, uint64_t id)
	{
		//TODO: Definitely change this!
		state.tags = &tags;
		state.sync_tags();

		if (video.is_open())
		{
			state.time_max = timestamp(std::chrono::duration_cast<std::chrono::seconds>(video.duration()));
		}
		else
		{
			state.time_min = timestamp{};
		}

		static int64_t first_frame = 0;
		static bool expanded = true;
		timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()) };

		std::string title = "Timeline##" + std::to_string(id);
		ImVec2 default_window_padding = ImGui::GetStyle().WindowPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin(title.c_str()))
		{
			ImGui::PushID(title.c_str());
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, default_window_padding);
			video_timeline(state, current_time, selected_timestamp, moving_timestamp, dirty_flag);
			ImGui::PopStyleVar();
			
			if (current_time.seconds_total != std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()))
			{
				video.seek(current_time.seconds_total);
			}
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, bool& dirty_flag)
	{
		if (ImGui::Begin("Tag Manager", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			widgets::tag_manager(tags, tag_rename, dirty_flag);
		}
		ImGui::End();
	}
}
