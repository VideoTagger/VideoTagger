#include "timeline_menu_popup.hpp"
#include "pch.hpp"

#include <core/app_context.hpp>
#include <events/tags/tag_change_display_request_event.hpp>

namespace vt::ui
{
	timeline_menu_popup::timeline_menu_popup(tag_storage* tags) : popup{ "Timeline Menu" }, tags_{ tags } {}

	void timeline_menu_popup::on_render()
	{
		if (ImGui::SmallButton("Show All"))
		{
			size_t tags_size = displayed_tags_.size();

			displayed_tags_.clear();
			for (const auto& tag : *tags_)
			{
				displayed_tags_.push_back(tag.name);
				ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag.name, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Hide All"))
		{
			for (const auto& tag : displayed_tags_)
			{
				ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag, false);
			}
			displayed_tags_.clear();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Toggle All"))
		{
			std::vector<std::string> new_tags;
			for (const auto& tag : *tags_)
			{
				if (std::find(displayed_tags_.begin(), displayed_tags_.end(), tag.name) != displayed_tags_.end())
				{
					ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag.name, false);
				}
				else
				{
					ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag.name, true);
					new_tags.push_back(tag.name);
				}
			}
			set_displayed_tags(new_tags);
		}

		if (ImGui::BeginChild("##TagList", { ImGui::GetContentRegionAvail().x, 150 }))
		{
			if (ImGui::BeginTable("##TagListTable", 1, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_PadOuterX))
			{
				auto& style = ImGui::GetStyle();
				for (const auto& tag : *tags_)
				{
					ImGui::TableNextColumn();
					auto it = std::lower_bound(displayed_tags_.begin(), displayed_tags_.end(), tag.name);
					bool visible = it != displayed_tags_.end() and *it == tag.name;

					auto name = (visible ? icons::visibility_on : icons::visibility_off) + std::string(" ") + tag.name;
					if (!visible) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
					if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
					{
						if (visible)
						{
							ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag.name, false);
							displayed_tags_.erase(it);
						}
						else
						{
							ctx_.dispatch_event<tag_change_display_request_event>("timeline", *tags_, tag.name, true);
							displayed_tags_.insert(it, tag.name);
						}
					}
					if (!visible) ImGui::PopStyleColor();
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
	}

	void timeline_menu_popup::set_tag_storage(tag_storage* tags)
	{
		tags_ = tags;
	}

	void timeline_menu_popup::set_displayed_tags(const std::vector<std::string>& displayed_tags)
	{
		displayed_tags_ = displayed_tags;
	}
	
	const std::vector<std::string>& timeline_menu_popup::displayed_tags() const
	{
		return displayed_tags_;
	}
}
