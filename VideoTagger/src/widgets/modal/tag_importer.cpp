#include "pch.hpp"
#include "tag_importer.hpp"
#include <core/app_context.hpp>
#include <widgets/controls.hpp>
#include <widgets/icons.hpp>
#include <utils/filesystem.hpp>

namespace vt::widgets::modal
{
	void tag_importer::open()
	{
		ImGui::OpenPopup("Import Tags");
	}

	bool tag_importer::load_tags()
	{
		//TODO: maybe dont add tags that are already in the project

		std::ifstream tags_file(tags_path);
		if (!tags_file.is_open())
		{
			return false;
		}

		auto json_tags = nlohmann::json::parse(tags_file);

		if (!json_tags.contains("tags"))
		{
			return false;
		}

		tag_storage loaded_tags = json_tags["tags"];

		for (auto& tag : loaded_tags)
		{
			imported_tags.push_back(imported_tag_data{ tag, true });
		}

		return true;
	}

	//TODO: selecting which tag to import when there're conflicts
	bool tag_importer::render(bool& is_open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
		auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		bool result = ImGui::BeginPopupModal("Import Tags", &is_open, flags);
		if (result)
		{
			auto& io = ImGui::GetIO();
			auto& style = ImGui::GetStyle();

			auto icon = icons::exit;
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::Text("Options");
			ImGui::PopFont();
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(icon).x - style.WindowPadding.x - style.WindowRounding);
			if (icon_button(icon))
			{
				is_open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();

			std::string path = tags_path.u8string();

			int input_flags = ImGuiInputTextFlags_AutoSelectAll;

			if (ImGui::InputTextWithHint("##ImportPath", "Tags Path...", &path, input_flags))
			{
				tags_path = std::filesystem::absolute(path);
				tags_path.make_preferred();
			}

			ImGui::SameLine();
			auto path_sel = fmt::format("{}##PathSelector", icons::dots_hor);
			if (ImGui::Button(path_sel.c_str()))
			{
				utils::dialog_filters filters{ { "VideoTagger Tags", "vttags" } };
				auto result = utils::filesystem::get_file({}, filters);
				if (result)
				{
					tags_path = result.path;
					tags_path.make_preferred();
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Load"))
			{
				if (!load_tags())
				{
					//TODO: popup or some text on the window;
					debug::log("File not found");
				}
			}

			bool update_all = false;
			bool update_state = false;

			static constexpr float tag_column_width = 100;


			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (icon_button(vt::icons::toggle_more))
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


			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
			bool is_scrollable_list_open = ImGui::BeginChild("##ScrollableTagList", ImGui::GetContentRegionAvail());

			//ImGui::PopStyleVar();
			if (is_scrollable_list_open)
			{
				static auto color_ref = imported_tags.end();
				int id{};

				auto node_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap;

				for (auto it = imported_tags.begin(); it != imported_tags.end(); ++it)
				{
					auto& is_selected = it->selected;
					auto& tag = it->tag;


					ImGui::PushID(id++);

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

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
					bool node_open = ImGui::TreeNodeEx("##TagManagerNode", node_flags);
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();

					auto icon = node_open ? icons::expand_less : icons::expand_more;

					ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
					ImGui::TextUnformatted(tag.name.c_str());
					ImGui::SameLine(ImGui::GetContentRegionMax().x - style.ItemSpacing.x - ImGui::CalcTextSize(icon).x);
					ImGui::TextUnformatted(icon);
					ImGui::SameLine(ImGui::GetContentRegionMax().x - style.ItemSpacing.x - ImGui::CalcTextSize(icon).x);

					//TODO: better checkbox position
					ImGui::Checkbox(it->tag.name.c_str(), &it->selected);

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

							ImGui::Text(tag.name.c_str());
							ImGui::NextColumn();
							ImGui::Text("Color");
							ImGui::NextColumn();
							ImGui::ColorButton("##ColorButton", color, ImGuiColorEditFlags_NoInputs);
							ImGui::Columns();
							ImGui::EndTable();
						}
						ImGui::PopStyleColor();
						ImGui::Indent();
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			ImGui::Dummy(style.ItemSpacing);
			if (ImGui::Button("Import"))
			{
				for (auto& tag_data : imported_tags)
				{
					if (!tag_data.selected)
					{
						continue;
					}

					auto& project_tags = ctx_.current_project->tags;

					if (project_tags.contains(tag_data.tag.name))
					{
						project_tags.at(tag_data.tag.name) = tag_data.tag;
					}
					else
					{
						project_tags.insert(tag_data.tag.name, tag_data.tag.color);
					}
				}

				result = true;
				is_open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel") or ImGui::IsKeyPressed(ImGuiKey_Escape, false))
			{
				is_open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
	
			if (!is_open)
			{
				tags_path.clear();
				imported_tags.clear();
			}
		}
		ImGui::PopStyleVar();

		return result;
	}
}
