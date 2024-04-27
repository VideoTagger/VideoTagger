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
	
	void draw_timeline_widget(const char* id, timeline_state& state, std::optional<selected_segment_data>& selected_timestamp, std::optional<moving_segment_data>& moving_timestamp, bool& dirty_flag, bool& open)
	{
		ImVec2 default_window_padding = ImGui::GetStyle().WindowPadding;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin(id, &open))
		{
			state.enabled = state.current_video_group_id != 0;
			
			ImGui::PushID(id);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, default_window_padding);
			
			video_timeline(state, selected_timestamp, moving_timestamp, dirty_flag);
			
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
