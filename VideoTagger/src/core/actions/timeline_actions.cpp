#include "pch.hpp"
#include "timeline_actions.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>

namespace vt
{
	timestamp_action::timestamp_action() : keybind_action(action_name) {}

	void timestamp_action::invoke() const
	{
		if (!ctx_.current_project.has_value() or ctx_.current_video_group_id() == invalid_video_group_id) return;

		auto& data = ctx_.insert_segment_data[tag_];
		data.start = ctx_.video_timeline.current_time_;
		data.end = ctx_.video_timeline.current_time_;
		data.tag = tag_;
		data.ready = true;
		data.show_insert_popup = tag_.empty();
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
		if (!ctx_.current_project.has_value() or ctx_.current_video_group_id() == invalid_video_group_id) return;

		auto& data = ctx_.insert_segment_data[tag_];
		data.tag = tag_;
		auto type = type_;
		if (type_ == segment_action_type::auto_)
		{
			if (data.start == std::nullopt)
			{
				type = segment_action_type::start;
			}
			else if (data.end == std::nullopt)
			{
				type = segment_action_type::end;
			}
		}

		switch (type)
		{
			case segment_action_type::start:
			{
				data.start = ctx_.video_timeline.current_time_;
			}
			break;
			case segment_action_type::end:
			{
				data.end = ctx_.video_timeline.current_time_;
			}
			break;
		}

		if (data.start != std::nullopt and data.end != std::nullopt)
		{
			data.ready = true;
			data.show_insert_popup = tag_.empty();
		}
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
