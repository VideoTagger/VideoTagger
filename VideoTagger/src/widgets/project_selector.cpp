#define _CRT_SECURE_NO_WARNINGS
#include "project_selector.hpp"
#include <sstream>
#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <SDL.h>

#include <core/debug.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>
#include <utils/string.hpp>
#include <utils/time.hpp>
#include "icons.hpp"
#include "buttons.hpp"

#include <core/app.hpp>

//Author: https://github.com/ocornut/imgui/issues/474#issuecomment-169480920
bool BeginButtonDropdown(const char* label, ImVec2 button_size)
{
	ImGui::SameLine(0.f, 0.f);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImVec2 cursor_pos = ImGui::GetCursorPos();

	ImVec2 size(20, button_size.y);
	bool pressed = ImGui::Button("##", size);

	// Arrow
	ImVec2 center(window->Pos.x + cursor_pos.x + 10, window->Pos.y + cursor_pos.y + button_size.y / 2);
	float r = 8.f;
	center.y -= r * 0.25f;
	ImVec2 a = center + ImVec2(0, 1) * r;
	ImVec2 b = center + ImVec2(-0.866f, -0.5f) * r;
	ImVec2 c = center + ImVec2(0.866f, -0.5f) * r;

	window->DrawList->AddTriangleFilled(a, b, c, ImGui::GetColorU32(ImGuiCol_Text));

	// Popup

	ImVec2 popup_pos;

	popup_pos.x = window->Pos.x + cursor_pos.x - button_size.x;
	popup_pos.y = window->Pos.y + cursor_pos.y + button_size.y;

	ImGui::SetNextWindowPos(popup_pos);

	if (pressed)
	{
		ImGui::OpenPopup(label);
	}

	if (ImGui::BeginPopup(label))
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_Button]);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Button]);
		return true;
	}

	return false;
}

void EndButtonDropdown()
{
	ImGui::PopStyleColor(3);
	ImGui::EndPopup();
}

namespace vt::widgets
{
	project_selector::project_selector(const std::vector<project>& projects) : projects_{ projects }
	{

	}

	void project_selector::render_project_creation_menu()
	{
		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		
		auto win_size = ImGui::GetContentRegionMax() * ImVec2(0.55f, 0.45f);
		ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);

		if (ImGui::BeginPopupModal("Project Configuration", nullptr, flags))
		{
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::LabelText("##ProjectCfgTitle", "Project Configuration");
			ImGui::Separator();
			ImGui::Dummy(style.ItemSpacing);
			ImGui::PopFont();
			ImGui::TextDisabled("Name");

			auto avail_size = ImGui::GetContentRegionAvail();
			auto input_width = avail_size.x * 0.9f;
			
			ImGui::SetNextItemWidth(input_width);
			ImGui::InputTextWithHint("##ProjectCfgName", "Project Name...", &temp_project.name, ImGuiInputTextFlags_AutoSelectAll);

			ImGui::TextDisabled("Location");
			std::string path = std::filesystem::absolute(temp_project.path).string();

			ImGui::SetNextItemWidth(input_width);
			if (ImGui::InputTextWithHint("##ProjectCfgPath", "Project Path...", &path, ImGuiInputTextFlags_AutoSelectAll))
			{
				temp_project.path = std::filesystem::absolute(path);
				temp_project.path.make_preferred();
			}
			ImGui::SameLine();
			auto path_sel = std::string(icons::dots_hor) + "##ProjectCfgPathSelector";
			if (ImGui::Button(path_sel.c_str()))
			{
				auto result = utils::filesystem::get_folder();
				if (result)
				{
					temp_project.path = result.path;
					temp_project.path.make_preferred();
				}
			}

			auto button_size = ImGui::CalcTextSize("Cancel") + style.ItemInnerSpacing * 2;
			button_size *= 1.15f;

			ImGui::SetCursorPosY(win_size.y - style.WindowPadding.y - button_size.y);

			bool valid = !temp_project.name.empty() and !temp_project.path.empty();

			valid &= std::filesystem::is_directory(temp_project.path) and std::filesystem::exists(temp_project.path);

			auto temp_project_copy = temp_project;
			temp_project_copy.path = (temp_project.path / temp_project.name).replace_extension(project::extension);

			auto it = std::find(projects_.begin(), projects_.end(), temp_project_copy);
			valid &= (it == projects_.end());

			if (!valid) ImGui::BeginDisabled();
			bool pressed = ImGui::Button("Create##ProjectCfg", button_size) || ImGui::IsKeyPressed(ImGuiKey_Enter);
			if (!valid) ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Cancel##ProjectCfg", button_size))
			{
				ImGui::CloseCurrentPopup();
			}			

			if (valid and pressed)
			{
				//TODO: Check if such project doesn't already exist
				temp_project = temp_project_copy;
				temp_project.save();
				projects_.push_back(temp_project);

				if (on_project_list_update == nullptr) return;
				on_project_list_update();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);
	}

	void project_selector::render_project_widget(size_t id, project& project)
	{
		ImVec2 size = { 0, ImGui::GetTextLineHeightWithSpacing() * 2 };
		auto imgui_id = static_cast<ImGuiID>(id);

		if (id == 0)
		{
			ImGui::TableSetupColumn("Project Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Modification Time", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoResize);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel);
			ImGui::TableHeadersRow();
		}
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::PushID(imgui_id);
		bool is_valid = project.is_valid();
		if (!is_valid)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Selectable("##ProjectListSelectable", false, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_SpanAllColumns, size) and on_click_project != nullptr)
		{
			on_click_project(project);
		}
		if (!is_valid)
		{
			ImGui::EndDisabled();
		}
		ImGui::PopID();

		ImGui::SameLine();

		ImGui::BeginGroup();
		std::string name = !project.name.empty() ? project.name : "- Invalid Project! -";
		ImGui::Text(name.c_str());
		auto path = project.path;
		std::optional<tm> mod_time = project.modification_time();
		if (!path.empty() and std::filesystem::exists(path))
		{
			path = std::filesystem::relative(path);
		}
		ImGui::TextDisabled(path.string().c_str());
		ImGui::EndGroup();

		ImGui::TableNextColumn();
		std::string time_str;
		std::string exact_time_str;
		if (mod_time.has_value())
		{
			std::tm mod_time_tm = mod_time.value();

			auto last_mod = utils::time::diff(std::time(nullptr), std::mktime(&mod_time_tm));
			time_str = utils::time::interval_str(last_mod);
			time_str = (time_str.empty() ? "Just now" : time_str + " ago");

			std::stringstream ss;
			ss << std::put_time(&mod_time.value(), "%d.%m.%Y %H:%M:%S");
			exact_time_str = ss.str();
		}
		
		ImGui::AlignTextToFramePadding();
		ImGui::Text(time_str.c_str());
		if (!exact_time_str.empty())
		{
			ImGui::SetItemTooltip(exact_time_str.c_str());
		}

		ImGui::TableNextColumn();
		ImGui::PushID(imgui_id);
		if (widgets::icon_button(icons::dots_hor, size))
		{
			ImGui::OpenPopup("##ProjectCtxMenu");
		}

		if (ImGui::BeginPopup("##ProjectCtxMenu"))
		{
			if (is_valid)
			{
				std::string menu_name = std::string(icons::folder) + " Open Containing Folder";
				if (ImGui::MenuItem(menu_name.c_str()))
				{
					auto path = std::filesystem::absolute(project.path.parent_path()).string();
					if (!path.empty())
					{
						std::string uri = "file://" + path;
						SDL_OpenURL(uri.c_str());
					}
				}
			}

			ImGui::Separator();
			{
				std::string menu_name = std::string(icons::visibility_off) + " Remove From List";
				if (ImGui::MenuItem(menu_name.c_str()))
				{
					projects_.erase(std::find(projects_.begin(), projects_.end(), project));
					if (on_project_list_update != nullptr) on_project_list_update();
				}
			}
			{
				std::string menu_name = std::string(icons::delete_) + " Delete";
				if (std::filesystem::is_regular_file(project.path) and project.path.extension() == std::string(".") + project::extension and ImGui::MenuItem(menu_name.c_str()))
				{
					const SDL_MessageBoxButtonData buttons[] = {
						// flags, buttonid, text
						{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
						{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Delete" }
					};

					SDL_MessageBoxData data{};
					data.flags = SDL_MESSAGEBOX_WARNING;

					//TODO: Replace title
					data.buttons = buttons;
					data.numbuttons = sizeof(buttons) / sizeof(buttons[0]);
					data.title = "VideoTagger";
					auto message = "Are you sure you want to delete the project file?\n\nFilepath:\n" + std::filesystem::absolute(project.path).string();
					data.message = message.c_str();
					int buttonid{};
					SDL_ShowMessageBox(&data, &buttonid);

					switch (buttonid)
					{
						case 1:
						{
							debug::log("Deleting project file: " + project.path.string());
							std::error_code ec{};
							if (std::filesystem::remove(project.path, ec))
							{
								projects_.erase(std::find(projects_.begin(), projects_.end(), project));
								if (on_project_list_update != nullptr) on_project_list_update();
							}
							else
							{
								debug::error("Project file couldn't be deleted: " + project.path.string());
								auto message = "Project file couldn't be deleted\n\nFilepath:\n" + project.path.string();
								message += "\nReason:\n" + ec.message() + "\nCode: " + std::to_string(ec.value());
								SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VideoTagger", message.c_str(), nullptr);
							}

						}
						break;
					}
				}
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}

	void project_selector::sort()
	{
		std::sort(projects_.begin(), projects_.end(), [](const project& left, const project& right)
		{
			auto ltm = left.modification_time();
			auto rtm = right.modification_time();
			if (ltm.has_value() and !rtm.has_value()) return true;
			else if (!ltm.has_value() and rtm.has_value()) return false;
			else if (!ltm.has_value() and !rtm.has_value()) return left.name < right.name;
			return std::mktime(&ltm.value()) > std::mktime(&rtm.value());
		});
	}

	void project_selector::remove(const project& project)
	{
		projects_.erase(std::find(projects_.begin(), projects_.end(), project));
	}

	void project_selector::load_projects_file(const std::filesystem::path& filepath)
	{
		if (!std::filesystem::exists(filepath)) return;
		auto json = utils::json::load_from_file(filepath);
		const auto& projects = json["projects"];
		if (!projects.is_array())
		{
			debug::error("Invalid projects list file structure");
			return;
		}
		auto list = projects.get<std::vector<std::filesystem::path>>();
		projects_.clear();
		projects_.resize(list.size());
		for (size_t i = 0; i < list.size(); ++i)
		{
			projects_[i] = project::load_from_file(list[i]);
		}
		if (on_project_list_update == nullptr) return;
		on_project_list_update();
	}

	void project_selector::save_projects_file(const std::filesystem::path& filepath)
	{
		nlohmann::ordered_json json;
		auto& projects = json["projects"];
		std::vector<std::filesystem::path> project_paths(projects_.size());
		for (size_t i = 0; i < projects_.size(); ++i)
		{
			project_paths[i] = std::filesystem::relative(projects_[i].path);
		}
		projects = project_paths;
		utils::json::write_to_file(json, filepath);
	}

	void project_selector::set_opened(bool value)
	{
		if (value and !ImGui::IsPopupOpen("Project Selector"))
		{
			ImGui::OpenPopup("Project Selector" , ImGuiPopupFlags_NoOpenOverExistingPopup);
		}
	}

	void project_selector::render()
	{
		constexpr auto rounding = 7.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, rounding);
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		auto io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Always);

		bool open_project_config = false;

		if (ImGui::BeginPopupModal("Project Selector", nullptr, flags))
		{
			if (ImGui::IsWindowAppearing())
			{
				sort();
			}
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::LabelText("##ProjectSelectorTitle", "Projects");
			ImGui::PopFont();
			ImGui::Dummy(ImGui::GetStyle().ItemSpacing);

			auto max_content_size = ImGui::GetContentRegionAvail();
			ImGui::SetNextItemWidth(max_content_size.x);
			ImGui::InputTextWithHint("##ProjectSelectorSearch", "Search...", &filter, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EscapeClearsAll);
						
			const auto& style = ImGui::GetStyle();
			auto panels_area = ImGui::GetContentRegionAvail() - style.WindowPadding;

			if (ImGui::BeginTable("##ProjectPanels", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
			{
				ImGui::TableSetupColumn(nullptr, 0, panels_area.x * 0.6f);
				ImGui::TableSetupColumn(nullptr, 0, panels_area.x * 0.3f);

				ImGui::TableNextColumn();
				{
					ImVec2 list_panel_size = ImGui::GetContentRegionAvail();
					std::vector<project> filtered_projects;
					if (!filter.empty())
					{
						auto tokens = utils::string::split(utils::string::to_lowercase(utils::string::trim_whitespace(filter)), ' ');
						for (const auto& project : projects_)
						{
							bool passes_filter = true;
							for (const auto& token : tokens)
							{
								auto ttoken = utils::string::trim_whitespace(token);
								std::string name = utils::string::to_lowercase(project.name);
								passes_filter &= name.find(ttoken) != std::string::npos;
							}

							if (passes_filter)
							{
								filtered_projects.push_back(project);
							}
						}
					}
					else
					{
						filtered_projects = projects_;
					}

					//list_size.y -= 2 * button_size.y + style.ItemSpacing.y + style.WindowPadding.y;
					if (ImGui::BeginChild("##Project List", list_panel_size))
					{
						if (!filtered_projects.empty())
						{
							if (ImGui::BeginTable("##ProjectListTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBody))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding * 0.5f);
								for (size_t i = 0; i < filtered_projects.size(); ++i)
								{
									render_project_widget(i, filtered_projects[i]);
								}
								ImGui::PopStyleVar();
								ImGui::EndTable();
							}
						}
						else
						{
							auto avail_area = ImGui::GetContentRegionMax();
							constexpr const char* text = "No Projects Found";
							auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
							ImGui::SetCursorPos(avail_area / 2 - half_text_size);
							ImGui::BeginDisabled();
							ImGui::TextWrapped(text);
							ImGui::EndDisabled();
						}
					}
					ImGui::EndChild();
				}

				ImGui::TableNextColumn();
				{
					ImVec2 button_panel_size = ImGui::GetContentRegionAvail();
					ImVec2 button_size = { button_panel_size.x - style.ItemInnerSpacing.x - 2 * style.WindowPadding.x, ImGui::GetTextLineHeightWithSpacing() * 2 };

					if (ImGui::BeginChild("##Project List Buttons", button_panel_size))
					{
						ImGui::Dummy(style.ItemSpacing);
						ImGui::BeginGroup();
						if (ImGui::Button("New Project", button_size))
						{
							open_project_config = true;
							ImGui::CloseCurrentPopup();
						}

						if (BeginButtonDropdown("##ProjectDropdown", button_size))
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
							if (ImGui::Button("Add Existing Project", button_size))
							{
								utils::dialog_filter filter{ "VideoTagger Project", project::extension };
								auto result = utils::filesystem::get_file({}, { filter });

								if (result)
								{
									auto it = std::find_if(projects_.begin(), projects_.end(), [result](const project& project)
									{
										return std::filesystem::absolute(project.path) == std::filesystem::absolute(result.path);
									});

									if (it == projects_.end())
									{
										projects_.push_back(project::load_from_file(result.path));
										if (on_project_list_update == nullptr) return;
										on_project_list_update();
									}
									else
									{
										std::string message = "Cannot add this project since it already exits.\nFilepath: " + std::filesystem::relative(result.path).string();
										//TODO: Change the title based on the app window
										SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "VideoTagger", message.c_str(), nullptr);
									}									
								}
								ImGui::CloseCurrentPopup();
							}
							ImGui::PopStyleColor();
							EndButtonDropdown();
						}
						ImGui::EndGroup();
					}
					ImGui::EndChild();
				}
				ImGui::EndTable();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		render_project_creation_menu();

		if (open_project_config)
		{
			temp_project = project{};
			ImGui::OpenPopup("Project Configuration", ImGuiPopupFlags_NoOpenOverExistingPopup);
		}
	}
}
