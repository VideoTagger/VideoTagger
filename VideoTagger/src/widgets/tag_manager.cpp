#include "tag_manager.hpp"

#include <algorithm>
#include <string>
#include <cmath>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_stdlib.h>

#include <utils/random.hpp>
#include "buttons.hpp"
#include "icons.hpp"
#include <core/app_context.hpp>

namespace vt::widgets
{
	constexpr ImGuiColorEditFlags color_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip;

	static bool add_tag_popup(tag_storage& tags, tag_storage::iterator& added_entry)
	{
		//TODO: Improve UI layout
		//TODO: Block "Done" when tag already exists

		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		auto flags = ImGuiWindowFlags_AlwaysAutoResize;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto win_size = ImVec2{ 290, 110 };
		ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);

		if (ImGui::BeginPopupModal("Add New Tag", nullptr, flags))
		{
			std::string error_text;
			static ImVec4 color{ 0, 0, 0, 1 };
			if (ImGui::IsWindowAppearing())
			{
				auto hue = utils::random::get<float>();
				auto value = utils::random::get<float>(0.5f, 1.0f);
				ImGui::ColorConvertHSVtoRGB(hue, 0.75f, value, color.x, color.y, color.z);
			}
			//I don't know if it's safe for this to be static
			static std::string tag_name;
			ImGui::Text("Tag Name");
			ImGui::SameLine();

			ImGui::SetNextItemWidth(100);
			ImGui::InputText("##TagName", &tag_name);

			tag_validate_result valid_tag_name = tags.validate_tag_name(tag_name);

			if (valid_tag_name != tag_validate_result::ok)
			{
				switch (valid_tag_name)
				{
				case vt::tag_validate_result::already_exists: error_text = "Already exists"; break;
				case vt::tag_validate_result::invalid_name: error_text = "Invalid name"; break;
				}
			}

			ImGui::SameLine();
			if (ImGui::BeginPopup("##ColorPicker"))
			{
				ImGui::ColorPicker3("##ColorPicker", reinterpret_cast<float*>(&color), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
				ImGui::EndPopup();
			}
			if (ImGui::ColorButton("##ColorPreview", color, color_button_flags))
			{
				ImGui::OpenPopup("##ColorPicker");
			}

			ImGui::BeginDisabled(valid_tag_name != tag_validate_result::ok);
			if (ImGui::Button("Done"))
			{
				auto [it, inserted] = tags.insert(tag_name);
				if (inserted)
				{
					it->color = ImGui::ColorConvertFloat4ToU32(color);
					added_entry = it;
				}
				
				tag_name.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();
			
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				tag_name.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::TextColored({ 0.9f, 0.05f, 0.05f, 1.0f }, error_text.c_str());
		
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return return_value;
	}

	bool tag_manager(tag_storage& tags, tag_storage::iterator& selected_entry, tag_manager_flags flags)
	{
		//TODO: Improve UI layout
		//TODO: Maybe extract some stuff into separate functions for better readability

		bool return_value = false;
		auto& style = ImGui::GetStyle();
		ImVec2 button_size = ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() };

		bool open_add_tag_popup = false;
		bool open_color_picker_popup = false;
		bool update_all = false;
		bool update_state = false;

		if (true /*ImGui::BeginTable("##TagManager", 2)*/)
		{
			static constexpr float tag_column_width = 100;
			float button_region_width = button_size.x + style.CellPadding.x;

			//ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			//ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			if (ImGui::SmallButton("Expand All"))
			{
				update_state = true;
				update_all = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Collapse All"))
			{
				update_state = false;
				update_all = true;
			}
			ImGui::Separator();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
			if (ImGui::BeginChild("##ScrollableTagList", ImGui::GetContentRegionAvail() - button_size - ImVec2{0, style.ItemSpacing.y + style.FramePadding.y}))
			{
				static auto color_ref = tags.end();
				int id{};

				auto node_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth;
				
				for (auto it = tags.begin(); it != tags.end(); ++it)
				{
					auto& tag = *it;

					//ImGui::TableNextColumn();
					ImGui::PushID(id++);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
					if (icon_button(icons::close))
					{
						tags.erase(tag.name);
						ImGui::PopStyleVar();
						ImGui::PopID();
						break;
					}
					ImGui::SameLine();
					ImGui::PopStyleVar();
					auto color = ImGui::ColorConvertU32ToFloat4(tag.color);
					bool open_color_picker = false;

					if (update_all)
					{
						ImGui::SetNextItemOpen(update_state);
					}
					bool node_open = ImGui::TreeNodeEx("##TagManagerNode", node_flags);

					ImGui::SameLine();
					ImGui::Text(tag.name.c_str());
					if (node_open)
					{
						ImGui::Columns(2, "##TagColumnSeparator");
						ImGui::Text("Name");
						ImGui::NextColumn();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

						//TODO: Add filtering & readd the tag with a new name since std::map is used as a container (why not std::vector??)
						if (ImGui::InputText("##TagNameInput", &tag.name, ImGuiInputTextFlags_AutoSelectAll))
						{
							ctx_.is_project_dirty = true;
						}
						ImGui::NextColumn();
						ImGui::Text("Color");
						ImGui::NextColumn();
						if (ImGui::ColorButton("##ColorButton", color, color_button_flags))
						{
							color_ref = it;
							open_color_picker = true;
						}
						ImGui::Columns();
						ImGui::TreePop();
					}
					ImGui::PopID();

					//ImGui::TableNextColumn();

					if (open_color_picker)
					{
						color_ref = it;
						ctx_.color_picker.set_color(ImGui::ColorConvertU32ToFloat4(tag.color));
						ctx_.color_picker.set_opened(true);
						//color_copy = ImGui::ColorConvertU32ToFloat4(tag.color);
					}
				}

				if (ctx_.color_picker.render("##TagColorPicker") and color_ref != tags.end())
				{
					color_ref->color = ImGui::ColorConvertFloat4ToU32(ctx_.color_picker.color());
				}
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();
			//ImGui::Dummy(ImGui::GetStyle().ItemSpacing);
			if (ImGui::Button("Add Tag", button_size))
			{
				open_add_tag_popup = true;
			}
			
			//ImGui::EndTable();
		}
		

		/*
		if (ImGui::BeginTable("##TagManager", 2))
		{
			static constexpr float tag_column_width = 100;
			float button_region_width = button_size.x + style.CellPadding.x;

			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, tag_column_width);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, button_region_width);

			if (ImGui::TableNextColumn())
			{
				if (ImGui::BeginTable("##TagManagerList", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY))
				{
					static float color_column_width = ImGui::CalcTextSize("Color").x;
					static float name_column_width = tag_column_width - color_column_width;

					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, name_column_width);
					ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, color_column_width);
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
												
						//I don't know if it's safe for this to be static
						static ImVec4 backup_color;
						open_color_picker_popup = ImGui::ColorButton(color_button_id.c_str(), color, color_button_flags, { ImGui::GetContentRegionAvail().x, color_picker_size.y });
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

				if (!(flags & tag_manager_flags::no_add))
				{
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (button_region_size.x / 2 - button_size.x / 2));
					if (ImGui::Button("Add", button_size))
					{
						open_add_tag_popup = true;

					}
				}
				if (!(flags & tag_manager_flags::no_remove))
				{
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (button_region_size.x / 2 - button_size.x / 2));
					if (ImGui::Button("Remove", button_size) and selected_entry != tags.end())
					{
						tags.erase(selected_entry);
						selected_entry = tags.end();
					}
				}
			}

			ImGui::EndTable();
		}
		*/

		if (open_add_tag_popup)
		{
			ImGui::OpenPopup("Add New Tag");
		}

		tag_storage::iterator added_entry = tags.end();
		add_tag_popup(tags, added_entry);

		return return_value;
	}
}
