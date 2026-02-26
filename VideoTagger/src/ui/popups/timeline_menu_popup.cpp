#include "timeline_menu_popup.hpp"
#include "timeline_menu_popup.hpp"
#include "pch.hpp"
#include "timeline_menu_popup.hpp"

namespace vt::ui
{
	timeline_menu_popup::timeline_menu_popup(tag_storage* tags) : popup{ "Timeline Menu" }, tags_{ tags }, tags_modified_{} {}

	void timeline_menu_popup::on_render()
	{
		tags_modified_ = false;

		if (ImGui::SmallButton("Show All"))
		{
			size_t tags_size = displayed_tags_.size();

			displayed_tags_.clear();
			for (const auto& tag : *tags_)
			{
				displayed_tags_.push_back(tag.name);
			}

			if (displayed_tags_.size() != tags_size)
			{
				tags_modified_ = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Hide All"))
		{
			if (!displayed_tags_.empty())
			{
				tags_modified_ = true;
			}

			displayed_tags_.clear();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Toggle All"))
		{
			std::vector<std::string> new_tags;
			for (const auto& tag : *tags_)
			{
				if (std::find(displayed_tags_.begin(), displayed_tags_.end(), tag.name) != displayed_tags_.end()) continue;
				new_tags.push_back(tag.name);
			}
			tags_modified_ = true;
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
							displayed_tags_.erase(it);
							tags_modified_ = true;
						}
						else
						{
							displayed_tags_.insert(it, tag.name);
							tags_modified_ = true;
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

	bool vt::ui::timeline_menu_popup::tags_modified() const
	{
		return tags_modified_;
	}
}
