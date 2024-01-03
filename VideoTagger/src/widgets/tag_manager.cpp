#include "tag_manager.hpp"

#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace vt::widgets
{
	static bool add_tag_popup(tag_storage& tags, tag_storage::iterator& added_entry)
	{
		//TODO: Improve UI layout
		//TODO: Block "Done" when tag already exists

		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		
		auto win_size = ImVec2{ 290, 110 };
		ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);

		if (ImGui::BeginPopupModal("Add New Tag", 0, flags))
		{
			//I don't know if it's safe for this to be static
			//TODO: Use std::string
			static char tag_name[64]{};
			ImGui::SetNextItemWidth(100);
			ImGui::InputText("Tag Name", tag_name, 64);

			tag_validate_result valid_tag_name = tags.validate_tag_name(tag_name);

			if (valid_tag_name != tag_validate_result::ok)
			{
				std::string error_text;
				switch (valid_tag_name)
				{
				case vt::tag_validate_result::already_exists:
					error_text = "Already exists";
					break;
				case vt::tag_validate_result::invalid_name:
					error_text = "Invalid name";
					break;
				default:
					break;
				}

				ImGui::SameLine();
				ImGui::TextColored({0.9f, 0.05f, 0.05f, 1.0f}, error_text.c_str());
			}

			ImGui::BeginDisabled(valid_tag_name != tag_validate_result::ok);
			if (ImGui::Button("Done"))
			{
				auto [it, inserted] = tags.insert(tag_name);
				if (inserted)
				{
					added_entry = it;
				}
				std::fill(std::begin(tag_name), std::end(tag_name), 0);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				std::fill(std::begin(tag_name), std::end(tag_name), 0);
				ImGui::CloseCurrentPopup();
			}
		
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return return_value;
	}

	bool tag_manager(tag_storage& tags, tag_storage::iterator& selected_entry)
	{
		//TODO: Improve UI layout
		//TODO: Maybe extract some stuff into separate functions for better readability

		bool return_value = false;
		auto& style = ImGui::GetStyle();
		static constexpr ImVec2 button_size = { 100, 50 };
		static constexpr ImVec2 color_picker_size = { 20, 20 };

		bool open_add_tag_popup = false;
		bool open_color_picker_popup = false;

		if (ImGui::BeginTable("##TagManager", 2))
		{
			float button_region_width = button_size.x + style.CellPadding.x;

			ImGui::TableSetupColumn(nullptr, 0, 0);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, button_region_width);

			if (ImGui::TableNextColumn())
			{
				if (ImGui::BeginTable("##TagManagerList", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed/*, color_picker_size.x*/);
					ImGui::TableHeadersRow();

					for (auto it = tags.begin(); it != tags.end(); ++it)
					{
						ImGui::TableNextColumn();

						if (ImGui::Selectable(it->name.c_str(), selected_entry == it, 0, { 0, color_picker_size.y }))
						{
							return_value = selected_entry != it;
							selected_entry = it;
						}
						
						ImGui::TableNextColumn();
						std::string color_button_id = std::string("##Tag_") + it->name + "_ColorButton";
						std::string color_picker_id = std::string("##Tag_") + it->name + "_ColorPicker";
						ImVec4 color = ImGui::ColorConvertU32ToFloat4(it->color);
						ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoAlpha;
						
						//I don't know if it's safe for this to be static
						static ImVec4 backup_color;
						open_color_picker_popup = ImGui::ColorButton(color_button_id.c_str(), color, flags, { ImGui::GetContentRegionAvail().x, color_picker_size.y });
						if (open_color_picker_popup)
						{
							ImGui::OpenPopup(color_picker_id.c_str());
							backup_color = color;
						}

						if (ImGui::BeginPopup(color_picker_id.c_str()))
						{
							ImGui::ColorPicker3("##ColorPicker", &backup_color.x, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
							if (ImGui::Button("Done"))
							{
								it->color = ImGui::ColorConvertFloat4ToU32(backup_color);
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel"))
							{
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}
						
					}

					ImGui::EndTable();
				}
			}

			if (ImGui::TableNextColumn())
			{
				auto button_region_size = ImGui::GetContentRegionAvail();

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (button_region_size.x / 2 - button_size.x / 2));
				if (ImGui::Button("Add", button_size))
				{
					open_add_tag_popup = true;

				}

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (button_region_size.x / 2 - button_size.x / 2));
				if (ImGui::Button("Remove", button_size) and selected_entry != tags.end())
				{
					tags.erase(selected_entry);
					selected_entry = tags.end();
				}
			}

			ImGui::EndTable();
		}

		if (open_add_tag_popup)
		{
			ImGui::OpenPopup("Add New Tag");
		}

		tag_storage::iterator added_entry = tags.end();
		add_tag_popup(tags, added_entry);

		return return_value;
	}
}
