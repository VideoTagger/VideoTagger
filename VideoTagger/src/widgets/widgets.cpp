#include "pch.hpp"
#include "widgets.hpp"

#include "video_timeline.hpp"
#include "controls.hpp"
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
	
	void draw_timeline_widget(timeline_state& state, std::optional<selected_segment_data>& selected_timestamp, std::optional<moving_segment_data>& moving_timestamp, bool& dirty_flag, uint64_t id, bool is_group_open, bool& open)
	{
		std::string title = "Timeline##" + std::to_string(id);
		ImVec2 default_window_padding = ImGui::GetStyle().WindowPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin(title.c_str(), &open))
		{
			ImGui::PushID(title.c_str());
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, default_window_padding);
			if (is_group_open)
			{
				video_timeline(state, selected_timestamp, moving_timestamp, dirty_flag);
			}
			else
			{
				centered_text("Open a video group to display its segments...", ImGui::GetContentRegionMax());
			}
			ImGui::PopStyleVar();
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, bool& dirty_flag, bool& open)
	{
		if (ImGui::Begin("Tag Manager", &open, ImGuiWindowFlags_NoScrollbar))
		{
			widgets::tag_manager(tags, tag_rename, dirty_flag);
		}
		ImGui::End();
	}
}
