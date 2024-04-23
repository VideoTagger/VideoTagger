#include "pch.hpp"
#include "timeline_actions.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>

namespace vt
{
	timestamp_action::timestamp_action() : keybind_action(action_name) {}

	void timestamp_action::invoke() const
	{
		if (!ctx_.current_project.has_value() or ctx_.current_video_group_id == 0)
		{
			return;
		}

		if (!tag_.empty())
		{
			auto& tag = ctx_.current_project->tags[tag_];
			auto& segments = ctx_.get_current_segment_storage()[tag.name];
			segments.insert(ctx_.timeline_state.current_time);
		}
		else
		{
			//TODO: Show tag selector window
		}
	}

	void timestamp_action::to_json(nlohmann::ordered_json& json) const
	{
		auto& tag_name = json["tag-name"];
		if (tag_.empty())
		{
			tag_name = nullptr;
		}
		else
		{
			tag_name = tag_;
		}
	}

	void timestamp_action::from_json(const nlohmann::ordered_json& json)
	{
		if (json.contains("tag-name"))
		{
			const auto& tag_name = json.at("tag-name");
			if (!tag_name.is_null())
			{
				tag_ = json.at("tag-name");
			}
		}
	}

	void timestamp_action::render_properties()
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

			tag_names.push_back(tag.name.c_str());
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

	segment_action::segment_action() : keybind_action(action_name), type_{ segment_action_type::auto_ } {}

	void segment_action::invoke() const
	{

	}

	void segment_action::to_json(nlohmann::ordered_json& json) const
	{
		auto& tag_name = json["tag-name"];
		if (tag_.empty())
		{
			tag_name = nullptr;
		}
		else
		{
			tag_name = tag_;
		}
		json["type"] = type_;
	}

	void segment_action::from_json(const nlohmann::ordered_json& json)
	{
		if (json.contains("tag-name"))
		{
			const auto& tag_name = json.at("tag-name");
			if (!tag_name.is_null())
			{
				tag_ = tag_name;
			}
		}

		if (json.contains("type"))
		{
			type_ = json.at("type");
		}
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
			tag_names.push_back(tag.name.c_str());
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
