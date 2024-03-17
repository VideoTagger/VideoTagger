#include "timeline_actions.hpp"
#include <imgui.h>
#include <core/app_context.hpp>
#include <widgets/controls.hpp>

namespace vt
{
	add_timestamp_action::add_timestamp_action() : keybind_action("Insert Timestamp") {}
	void add_timestamp_action::invoke() const
	{

	}

	void add_timestamp_action::render_properties(bool compact)
	{
		if (!compact)
		{
			ImGui::TableNextColumn();
			ImGui::Text("Tag Name");
			ImGui::TableNextColumn();
		}
		else ImGui::SameLine();
		const auto& tags = ctx_.current_project->tags;
		int selected_tag{};
		std::vector<const char*> tag_names{ "Ask Later" };
		int i = 1;
		for (const auto& tag : tags)
		{
			if (tag_ == tag.name)
			{
				selected_tag = i;
			}
			tag_names.push_back(_strdup(tag.name.c_str()));
			++i;
		}

		if (ImGui::Combo("##TagName", &selected_tag, tag_names.data(), static_cast<int>(tag_names.size())))
		{
			if (selected_tag != 0)
			{
				tag_ = tag_names[selected_tag];
			}
			else
			{
				tag_.clear();
			}
		}
		ImGui::SameLine();
		widgets::help_marker("Choosing \"Ask Later\" will display a window, where you will have to select the tag");
	}
}
