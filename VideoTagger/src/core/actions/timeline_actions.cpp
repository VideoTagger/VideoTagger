#include "pch.hpp"
#include "timeline_actions.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>
#include <utils/random.hpp>

#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>

namespace vt
{
	timeline_action::timeline_action(const std::string& name) : keybind_action(name) {}

	const std::optional<std::string>& timeline_action::tag() const
	{
		return tag_;
	}

	void timeline_action::to_json(nlohmann::ordered_json& json) const
	{
		auto& tag_name = json["tag-name"];
		if (tag_.has_value())
		{
			tag_name = *tag_;
		}
		else
		{
			tag_name = nullptr;
		}
	}

	void timeline_action::from_json(const nlohmann::ordered_json& json)
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

	timestamp_action::timestamp_action() : timeline_action(action_name) {}

	void timestamp_action::invoke() const
	{
		if (!ctx_.current_project.has_value() or ctx_.current_video_group_id() == invalid_video_group_id) return;

		auto& segments = ctx_.get_current_segment_storage();
		auto current_timestamp = ctx_.displayed_videos.current_timestamp_as_timestamp();

		ctx_.dispatch_event<segment_insert_request_event>("keybind", segments, tag_, current_timestamp, !tag_.has_value(), false);
	}

	void timestamp_action::to_json(nlohmann::ordered_json& json) const
	{
		timeline_action::to_json(json);
	}

	void timestamp_action::from_json(const nlohmann::ordered_json& json)
	{
		timeline_action::from_json(json);
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
				tag_ = std::nullopt;
			}
		}

		ImGui::SameLine();
		ui::help_marker("Choosing \"Ask Later\" will display a window, where you will have to select the tag");
	}

	segment_action::segment_action() : timeline_action(action_name), type_{ segment_action_type::auto_ } {}

	void segment_action::invoke() const
	{
		if (!ctx_.current_project.has_value() or ctx_.current_video_group_id() == invalid_video_group_id) return;

		auto mark_it = ctx_.find_insert_segment_mark_by_tag(tag_);

		auto type = type_;
		if (type_ == segment_action_type::auto_)
		{
			type = mark_it == ctx_.insert_segment_marks.end() ? segment_action_type::start : segment_action_type::end;
		}

		auto& segments = ctx_.get_current_segment_storage();
		auto current_timestamp = ctx_.displayed_videos.current_timestamp_as_timestamp();

		switch (type)
		{
			case segment_action_type::start:
			{
				if (mark_it != ctx_.insert_segment_marks.end()) return;

				ctx_.dispatch_event<segment_insert_mark_start>("keybind", utils::random::get_uuid(), segments, tag_, current_timestamp);
			}
			break;
			case segment_action_type::end:
			{
				if (mark_it == ctx_.insert_segment_marks.end()) return;

				ctx_.dispatch_event<segment_insert_mark_end>("keybind", mark_it->mark_id, segments, current_timestamp, !tag_.has_value());
			}
			break;
		}
	}

	void segment_action::to_json(nlohmann::ordered_json& json) const
	{
		timeline_action::to_json(json);
		json["type"] = type_;
	}

	void segment_action::from_json(const nlohmann::ordered_json& json)
	{
		timeline_action::from_json(json);
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
				tag_ = std::nullopt;
			}
		}

		ImGui::SameLine();
		ui::help_marker("Choosing \"Ask Later\" will display a window, where you will have to select the tag");

		ImGui::TableNextColumn();
		ImGui::Text("Type");
		ImGui::TableNextColumn();

		int* selected_type = reinterpret_cast<int*>(&type_);
		static const char* types[]{ "Auto", "Start", "End" };
		ImGui::Combo("##Type", selected_type, types, sizeof(types) / sizeof(types[0]));

		ImGui::SameLine();
		ui::help_marker("Choosing \"Auto\" will automatically detect whether the segment should start or end");
	}
}
