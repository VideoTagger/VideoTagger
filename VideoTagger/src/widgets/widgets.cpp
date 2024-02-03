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
	
	void draw_timeline_widget_sample(video& video, tag_storage& tags, std::optional<selected_timestamp_data>& selected_timestamp, bool& dirty_flag, uint32_t id)
	{
		//TODO: Definitely change this!
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

		static int64_t first_frame = 0;
		static bool expanded = true;
		timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()) };

		std::string title = "Timeline##" + std::to_string(id);
		ImVec2 default_window_padding = ImGui::GetStyle().WindowPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin(title.c_str()))
		{
			ImGui::PushID(id);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, default_window_padding);
			video_timeline(test_timeline, current_time, selected_timestamp, dirty_flag);
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

	void draw_tag_manager_widget(tag_storage& tags)
	{
		if (ImGui::Begin("Tag Manager", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			static tag_storage::iterator selected = tags.end();
			if (widgets::tag_manager(tags, selected))
			{
				
			}
		}
		ImGui::End();
	}
}
