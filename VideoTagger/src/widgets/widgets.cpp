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
	

	void draw_tag_manager_widget(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, std::optional<tag_delete_data>& tag_delete, bool& dirty_flag, bool& open)
	{
		if (ImGui::Begin(tag_manager_window_name().c_str(), &open, ImGuiWindowFlags_NoScrollbar))
		{
			widgets::tag_manager(tags, tag_rename, tag_delete, dirty_flag);
		}
		ImGui::End();
	}
}
