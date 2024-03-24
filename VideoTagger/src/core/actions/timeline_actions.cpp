#include "timeline_actions.hpp"
#include <imgui.h>
#include <core/app_context.hpp>
#include <widgets/controls.hpp>

namespace vt
{
	add_timestamp_action::add_timestamp_action() : keybind_action("Insert Timestamp") {}
	void add_timestamp_action::invoke() const
	{
		if (!tag_.empty())
		{
			auto& tag = ctx_.current_project->tags[tag_];
			auto& segments = ctx_.current_project->segments[tag.name];
			//TODO: Change this to insert the timestamp at the timeline's cursor
			segments.insert(timestamp{});
		}
		else
		{
			//TODO: Show tag selector window
		}
	}

	void add_timestamp_action::render_properties()
	{
		ImGui::TableNextColumn();
		ImGui::Text("Tag Name");
		ImGui::TableNextColumn();

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

	segment_action::segment_action() : keybind_action("Start/End Segment"), type_{ segment_action_type::auto_ } {}
	void segment_action::invoke() const
	{

	}

	void segment_action::render_properties()
	{
		ImGui::TableNextColumn();
		ImGui::Text("Tag Name");
		ImGui::TableNextColumn();

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

		ImGui::TableNextColumn();
		ImGui::Text("Type");
		ImGui::TableNextColumn();

		int* selected_type = reinterpret_cast<int*>(&type_);
		static const char* types[]{ "Auto", "Start", "End" };
		ImGui::Combo("##Type", selected_type, types, sizeof(types) / sizeof(types[0]));

		ImGui::SameLine();
		widgets::help_marker("Choosing \"Auto\" will automatically detect whether the segment should start or end");
	}
}
