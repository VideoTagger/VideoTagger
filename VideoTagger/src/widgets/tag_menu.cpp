#include "pch.hpp"
#include "tag_menu.hpp"

#include "icons.hpp"
#include <core/app_context.hpp>

namespace vt::widgets
{
	bool tag_menu(tag_storage& tags, std::vector<std::string>& visible_tags, bool& tags_modifed)
	{
		bool result{};
		bool hide_popup = false;
		if (ImGui::SmallButton("Show All"))
		{
			size_t tags_size = visible_tags.size();

			visible_tags.clear();
			for (const auto& tag : tags)
			{
				visible_tags.push_back(tag.name);
			}
			result = true;

			if (visible_tags.size() != tags_size)
			{
				tags_modifed = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Hide All"))
		{
			if (!visible_tags.empty())
			{
				tags_modifed = true;
			}

			visible_tags.clear();
			result = true;
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Toggle All"))
		{
			std::vector<std::string> new_tags;
			for (const auto& tag : tags)
			{
				if (std::find(visible_tags.begin(), visible_tags.end(), tag.name) != visible_tags.end()) continue;
				new_tags.push_back(tag.name);
				tags_modifed = true;
			}
			visible_tags = new_tags;
			result = true;
		}

		if (ImGui::BeginChild("##TagList", { ImGui::GetContentRegionAvail().x, 150}))
		{
			if (ImGui::BeginTable("##TagListTable", 1, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_PadOuterX))
			{
				auto& style = ImGui::GetStyle();
				for (const auto& tag : tags)
				{
					ImGui::TableNextColumn();
					auto it = std::find(visible_tags.begin(), visible_tags.end(), tag.name);
					bool visible = it != visible_tags.end();
					
					auto name = (visible ? icons::visibility_on : icons::visibility_off) + std::string(" ") + tag.name;
					if (!visible) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
					if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
					{
						if (visible)
						{
							visible_tags.erase(it);
							tags_modifed = true;
						}
						else
						{
							visible_tags.push_back(tag.name);
							tags_modifed = true;
						}
					}
					if (!visible) ImGui::PopStyleColor();
				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		return result;
	}
}
