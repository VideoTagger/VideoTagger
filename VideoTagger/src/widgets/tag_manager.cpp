#include "pch.hpp"
#include "tag_manager.hpp"

#include <utils/random.hpp>
#include "controls.hpp"
#include "icons.hpp"
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <utils/string.hpp>

namespace vt::widgets
{
	constexpr ImGuiColorEditFlags color_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip;

	static bool add_tag_popup(tag_storage& tags, tag_storage::iterator& added_entry)
	{
		//TODO: Improve UI layout
		
		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		auto flags = ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_AlwaysAutoResize;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		auto win_size = ImVec2{ 390, 110 };
		ImGui::SetNextWindowSize(win_size, ImGuiCond_Appearing);

		if (ImGui::BeginPopupModal("Add New Tag", nullptr, flags))
		{
			std::string error_text;
			static ImVec4 color{ 0, 0, 0, 1 };
			bool is_appearing = ImGui::IsWindowAppearing();

			if (is_appearing)
			{
				auto hue = utils::random::get<float>();
				auto value = utils::random::get<float>(0.5f, 1.0f);
				ImGui::ColorConvertHSVtoRGB(hue, 0.75f, value, color.x, color.y, color.z);
			}

			//I don't know if it's safe for this to be static
			static std::string tag_name;
			if (is_appearing)
			{
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::Text("Tag Name");
			ImGui::SameLine();

			ImGui::SetNextItemWidth(200);
			ImGui::InputText("##TagName", &tag_name);

			tag_validate_result valid_tag_name = tags.validate_tag_name(tag_name);

			if (valid_tag_name != tag_validate_result::ok)
			{
				switch (valid_tag_name)
				{
					case vt::tag_validate_result::already_exists: error_text = "Already exists"; break;
					case vt::tag_validate_result::invalid_name: error_text = "Invalid name"; break;
					case vt::tag_validate_result::too_long: error_text = fmt::format("Name can be at most {} characters long", tag_storage::max_tag_name_length); break;
					default: error_text = "Invalid name"; break;
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
					return_value = true;
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

	bool rename_tag_popup(const std::string& id, const tag_rename_data& data, bool& pressed_button)
	{
		//TODO: improve layout

		static constexpr ImVec2 button_size = { 55, 30 };

		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (ImGui::BeginPopupModal(id.c_str(), nullptr, flags))
		{
			ImGui::Text("Are you sure you want to rename the tag \"%s\" to \"%s\"?", data.old_name.c_str(), data.new_name.c_str());
			//TODO: CHANGE THIS
			//ImGui::TextDisabled("ONE SHALL NOT PERFORM SUCH ACTION CARELESSLY FOR IT COULD BRING DIRE CONSEQUENCES");
			ImGui::NewLine();
			auto area_size = ImGui::GetWindowSize();

			ImGui::SetCursorPosX(area_size.x / 2 - button_size.x - 20);
			if (ImGui::Button("Yes", button_size))
			{
				pressed_button = true;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(area_size.x / 2 + 20);
			if (ImGui::Button("No", button_size) or ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				pressed_button = false;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return return_value;
	}

	bool delete_popup(const std::string& id, const std::string& tag_name, bool& pressed_yes)
	{
		static constexpr ImVec2 button_size = { 55, 30 };

		bool return_value = false;

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		if (ImGui::BeginPopupModal(id.c_str(), nullptr, flags))
		{
			ImGui::Text("Are you sure you want to delete tag \"%s\"", tag_name.c_str());

			ImGui::TextColored({ 1.f, 170.f / 255.f, 50.f / 255.f, 1.f }, "All segments associated with this tag will be deleted as well!");
			ImGui::NewLine();
			auto area_size = ImGui::GetWindowSize();

			ImGui::SetCursorPosX(area_size.x / 2 - button_size.x - style.ItemSpacing.x);
			if (ImGui::Button("Yes", button_size))
			{
				return_value = true;
				pressed_yes = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", button_size))
			{
				return_value = true;
				pressed_yes = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar(2);

		return return_value;
	}

	bool tag_manager(tag_storage& tags, std::optional<tag_rename_data>& tag_rename, std::optional<tag_delete_data>& tag_delete, bool& dirty_flag, tag_manager_flags flags)
	{
		//TODO: Maybe extract some stuff into separate functions for better readability

		bool return_value = false;
		auto& style = ImGui::GetStyle();

		bool open_add_tag_popup = false;
		bool open_rename_tag_popup = false;
		bool open_delete_tag_popup = false;
		bool update_all = false;
		bool update_state = false;

		if (true /*ImGui::BeginTable("##TagManager", 2)*/)
		{
			static constexpr float tag_column_width = 100;

			//ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			//ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			static std::string filter;
			if (ImGui::IsWindowAppearing())
			{
				filter.clear();
			}

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (icon_button(icons::add))
			{
				open_add_tag_popup = true;
			}
			tooltip("Add Tag");
			ImGui::SameLine();
			search_bar("##VideoGroupBrowserSearch", "Search...", filter, ImGui::GetContentRegionAvail().x - 2 * (ImGui::CalcTextSize(icons::toggle_less).x + 2 * style.FramePadding.x));
			ImGui::SameLine();
			if (icon_button(icons::toggle_more))
			{
				update_state = true;
				update_all = true;
			}
			tooltip("Expand All");
			ImGui::SameLine();
			ImGui::PopStyleVar();
			if (icon_button(icons::toggle_less))
			{
				update_state = false;
				update_all = true;
			}
			tooltip("Collapse All");
			ImGui::Separator();

			std::vector<std::string> tokens;
			if (!filter.empty())
			{
				tokens = utils::string::split(utils::string::to_lowercase(utils::string::trim_whitespace(filter)), ' ');
			}

			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
			bool is_scrollable_list_open = ImGui::BeginChild("##ScrollableTagList", ImGui::GetContentRegionAvail());
			
			//ImGui::PopStyleVar();
			if (is_scrollable_list_open)
			{
				size_t filter_passes{};
				static auto color_ref = tags.end();
				int id{};

				auto node_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth;
				
				static std::string tag_name;

				for (auto it = tags.begin(); it != tags.end();)
				{
					auto& tag = *it;

					bool passes_filter = true;
					for (const auto& token : tokens)
					{
						auto ttoken = utils::string::trim_whitespace(token);
						std::string name = utils::string::to_lowercase(tag.name);
						passes_filter &= name.find(ttoken) != std::string::npos;
					}

					if (!passes_filter)
					{
						++it;
						continue;
					}
					++filter_passes;


					//ImGui::TableNextColumn();
					ImGui::PushID(id++);
					/*
					if (icon_button(icons::close))
					{
						tags.erase(tag.name);
						ctx_.is_project_dirty = true;
						ImGui::PopStyleVar();
						ImGui::PopID();
						break;
					}
					*/
					auto color = ImGui::ColorConvertU32ToFloat4(tag.color);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x * 0.5f);
					ImGui::AlignTextToFramePadding();
					ImGui::TextColored(color, icons::label);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
					ImGui::SameLine();
					
					bool open_color_picker = false;

					if (update_all)
					{
						ImGui::SetNextItemOpen(update_state);
					}

					//A bit of a hack to not render the arrow
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
					bool node_open = ImGui::TreeNodeEx("##TagManagerNode", node_flags);
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
					{
						utils::drag_drop::set_payload("Tag", tag.name.c_str());
						ImGui::TextColored(color, icons::label);
						ImGui::SameLine();
						ImGui::TextUnformatted(tag.name.c_str());
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginPopupContextItem("##TagCtxMenu"))
					{
						std::string menu_name = fmt::format("{} Delete", icons::delete_);
						if (ImGui::MenuItem(menu_name.c_str()))
						{
							tag_delete_data delete_data;
							delete_data.tag = it->name;
							tag_delete = delete_data;

							open_delete_tag_popup = true;
						}
						ImGui::EndPopup();
					}
					if (ImGui::IsItemHovered() and ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("##TagCtxMenu");
					}

					auto icon = node_open ? icons::expand_less : icons::expand_more;

					ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
					ImGui::TextUnformatted(tag.name.c_str());
					ImGui::SameLine(ImGui::GetContentRegionMax().x - style.ItemSpacing.x - ImGui::CalcTextSize(icon).x);
					ImGui::TextUnformatted(icon);

					if (node_open)
					{
						ImGui::Unindent();
						ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
						if (ImGui::BeginTable("##Background", 1, ImGuiTableFlags_RowBg))
						{
							ImGui::TableNextColumn();
							ImGui::Columns(2, "##TagColumnSeparator");
							ImGui::Text("Name");
							ImGui::NextColumn();
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

							//TODO: Add filtering & readd the tag with a new name since std::map is used as a container (why not std::vector??)
							
							tag_name = tag.name;
							if (ImGui::InputText("##TagNameInput", &tag_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
							{
								tag_rename = tag_rename_data{ false, false, tag.name, tag_name };
								open_rename_tag_popup = true;
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
							ImGui::EndTable();
						}		
						ImGui::PopStyleColor();
						ImGui::Indent();
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

					if (it != tags.end())
					{
						++it;
					}
				}

				if (filter_passes == 0)
				{
					centered_text("No matching tags found...", ImGui::GetContentRegionAvail());
				}

				if (ctx_.color_picker.render("##TagColorPicker") and color_ref != tags.end())
				{
					color_ref->color = ImGui::ColorConvertFloat4ToU32(ctx_.color_picker.color());
					ctx_.is_project_dirty = true;
				}
			}
			ImGui::EndChild();
		}
		
		if (open_add_tag_popup)
		{
			ImGui::OpenPopup("Add New Tag");
		}

		if (open_rename_tag_popup)
		{
			ImGui::OpenPopup("Rename Tag");
		}

		if (open_delete_tag_popup)
		{
			ImGui::OpenPopup("Delete Tag");
		}

		tag_storage::iterator added_entry = tags.end();
		if (add_tag_popup(tags, added_entry))
		{
			dirty_flag = true;
		}

		//TODO: do this in app.cpp
		bool pressed_button{};
		
		if (tag_rename.has_value() and rename_tag_popup("Rename Tag", *tag_rename, pressed_button))
		{
			if (pressed_button == true)
			{
				tag_rename->ready = true;
			}
			else
			{
				tag_rename.reset();
			}
		}

		static bool pressed_yes = false;
		if (tag_delete.has_value() and delete_popup("Delete Tag", tag_delete->tag, pressed_yes))
		{
			if (pressed_yes)
			{
				tag_delete->ready = true;
			}
			else
			{
				tag_delete.reset();
			}
		}

		return return_value;
	}
}
