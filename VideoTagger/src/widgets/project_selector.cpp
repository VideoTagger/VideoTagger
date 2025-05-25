#include "pch.hpp"
#include "project_selector.hpp"

#include <core/debug.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>
#include <utils/string.hpp>
#include <utils/time.hpp>
#include <ui/icons.hpp>
#include "controls.hpp"

#include <core/app.hpp>

namespace vt::widgets
{
	project_selector::project_selector(const std::vector<project_info>& projects) : projects_{ projects } {}

	void project_selector::render_project_creation_menu()
	{
		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		
		auto win_size = ImGui::GetContentRegionMax() * ImVec2(0.55f, 0.45f);
		ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);

		if (ImGui::BeginPopupModal("Project Configuration", nullptr, flags))
		{
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::LabelText("##ProjectCfgTitle", "%s", ctx_.lang->get("project.configuration").c_str());
			ImGui::Separator();
			ImGui::Dummy(style.ItemSpacing);
			ImGui::PopFont();
			ImGui::TextDisabled("%s", ctx_.lang->get("name").c_str());

			auto avail_size = ImGui::GetContentRegionAvail();
			auto input_width = avail_size.x * 0.9f;
			
			ImGui::SetNextItemWidth(input_width);
			std::string proj_name_hint = fmt::format("{}...", ctx_.lang->get("project.name").c_str());
			ImGui::InputTextWithHint("##ProjectCfgName", proj_name_hint.c_str(), &temp_project.name, ImGuiInputTextFlags_AutoSelectAll);

			ImGui::TextDisabled("%s", ctx_.lang->get("location").c_str());
			std::string path = std::filesystem::absolute(temp_project.path).replace_extension().u8string();

			int input_flags = ImGuiInputTextFlags_AutoSelectAll;
			if (path_from_name)
			{
				temp_project.path.replace_filename(temp_project.name);
			}

			ImGui::SetNextItemWidth(input_width);

			if (ImGui::InputTextWithHint("##ProjectCfgPath", "Project Path...", &path, input_flags))
			{
				temp_project.path = std::filesystem::absolute(path);
				temp_project.path.make_preferred();
			}

			ImGui::SameLine();
			auto path_sel = fmt::format("{}##ProjectCfgPathSelector", icons::dots_hor);
			if (ImGui::Button(path_sel.c_str()))
			{
				utils::dialog_filters filters{ { "VideoTagger Project", project_info::extension } };
				auto result = utils::filesystem::save_file({}, filters, temp_project.name);
				if (result)
				{
					if (path_from_name)
					{
						result.path = result.path.replace_filename(temp_project.name);
					}
					temp_project.path = result.path;
					temp_project.path.make_preferred();
				}
			}
			ImGui::Checkbox(ctx_.lang->get("derive_filename_from_proj_name").c_str(), &path_from_name);

			auto button_size = ImGui::CalcTextSize(ctx_.lang->get("cancel").c_str()) + style.ItemInnerSpacing * 2;
			button_size *= 1.15f;

			ImGui::SetCursorPosY(win_size.y - style.WindowPadding.y - button_size.y);

			bool valid = !temp_project.name.empty() and !temp_project.path.empty();

			auto parent_path = temp_project.path.parent_path();
			valid &= std::filesystem::is_directory(parent_path) and std::filesystem::exists(parent_path);

			project_info temp_project_copy = temp_project;
			temp_project_copy.path = temp_project.path.replace_extension(project::extension);

			auto it = std::find(projects_.begin(), projects_.end(), temp_project_copy);
			valid &= (it == projects_.end());

			if (!valid) ImGui::BeginDisabled();
			bool pressed = ImGui::Button(ctx_.lang->get("create").c_str(), button_size) || ImGui::IsKeyPressed(ImGuiKey_Enter);
			if (!valid) ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button(ctx_.lang->get("cancel").c_str(), button_size))
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

	void project_selector::render_project_widget(size_t id, project_info& project)
	{
		ImVec2 size = { 0, ImGui::GetTextLineHeightWithSpacing() * 2 };
		auto imgui_id = static_cast<ImGuiID>(id);

		if (id == 0)
		{
			ImGui::TableSetupColumn(ctx_.lang->get("project.name").c_str(), ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(ctx_.lang->get("modification_time").c_str(), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoResize);
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
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		if (!is_valid)
		{
			ImGui::EndDisabled();
		}
		ImGui::PopID();

		ImGui::SameLine();

		ImGui::BeginGroup();
		std::string name = !project.name.empty() ? project.name : fmt::format("- {}! -", ctx_.lang->get("project.invalid").c_str());
		ImGui::TextUnformatted(name.c_str());
		auto path = project.path;
		std::optional<tm> mod_time = project.modification_time();
		if (!path.empty() and std::filesystem::exists(path))
		{
			path = std::filesystem::absolute(path);
		}
		ImGui::TextDisabled("%s", path.string().c_str());
		tooltip(path.string().c_str());
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
		ImGui::TextUnformatted(time_str.c_str());
		if (!exact_time_str.empty())
		{
			ImGui::SetItemTooltip("%s", exact_time_str.c_str());
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
				std::string menu_name = fmt::format("{} {}", icons::folder, ctx_.lang->get("show_in_explorer").c_str());
				if (ImGui::MenuItem(menu_name.c_str()))
				{
					auto path = std::filesystem::absolute(project.path.parent_path()).u8string();
					if (!path.empty())
					{
						utils::filesystem::open_in_explorer(path);
					}
				}
			}

			ImGui::Separator();
			{
				std::string menu_name = fmt::format("{} Remove From List", icons::visibility_off);
				if (ImGui::MenuItem(menu_name.c_str()))
				{
					projects_.erase(std::find(projects_.begin(), projects_.end(), project));
					if (on_project_list_update != nullptr) on_project_list_update();
				}
			}
			{
				std::string menu_name = fmt::format("{} Delete", icons::delete_);
				if (std::filesystem::is_regular_file(project.path) and project.path.extension() == std::string(".") + project::extension and ImGui::MenuItem(menu_name.c_str()))
				{
					const SDL_MessageBoxButtonData buttons[] = {
						// flags, buttonid, text
						{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, ctx_.lang->get("cancel").c_str() },
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
							debug::log("Deleting project file: {}", project.path.u8string());
							std::error_code ec{};
							if (std::filesystem::remove(project.path, ec))
							{
								projects_.erase(std::find(projects_.begin(), projects_.end(), project));
								if (on_project_list_update != nullptr) on_project_list_update();
							}
							else
							{
								debug::error("Project file couldn't be deleted: {}", project.path.u8string());
								auto message = "Project file couldn't be deleted\n\nFilepath:\n" + project.path.u8string();
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
		std::sort(projects_.begin(), projects_.end(), [](const project_info& left, const project_info& right)
		{
			auto ltm = left.modification_time();
			auto rtm = right.modification_time();
			if (ltm.has_value() and !rtm.has_value()) return true;
			else if (!ltm.has_value() and rtm.has_value()) return false;
			else if (!ltm.has_value() and !rtm.has_value()) return left.name < right.name;
			return std::mktime(&ltm.value()) > std::mktime(&rtm.value());
		});
	}

	void project_selector::remove(const project_info& project)
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
			projects_[i] = project_info::load_from_file(list[i]);
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
			project_paths[i] = std::filesystem::absolute(projects_[i].path);
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
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
		auto& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Always);

		bool open_project_config = false;

		if (ImGui::BeginPopupModal("Project Selector", nullptr, flags))
		{
			if (ImGui::IsWindowAppearing())
			{
				sort();
			}
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::LabelText("##ProjectSelectorTitle", "%s", ctx_.lang->get("projects").c_str());
			ImGui::PopFont();
			ImGui::Dummy(ImGui::GetStyle().ItemSpacing);

			search_bar("##ProjectSelectorSearch", ctx_.lang->get("search_hint").c_str(), filter);
						
			const auto& style = ImGui::GetStyle();
			auto panels_area = ImGui::GetContentRegionAvail() - style.WindowPadding;

			if (ImGui::BeginTable("##ProjectPanels", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
			{
				ImGui::TableSetupColumn(nullptr, 0, panels_area.x * 0.6f);
				ImGui::TableSetupColumn(nullptr, 0, panels_area.x * 0.3f);

				ImGui::TableNextColumn();
				{
					ImVec2 list_panel_size = ImGui::GetContentRegionAvail();
					std::vector<project_info> filtered_projects;
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
						if (ImGui::Button(ctx_.lang->get("project.new").c_str(), button_size))
						{
							open_project_config = true;
							ImGui::CloseCurrentPopup();
						}

						if (widgets::begin_button_dropdown("##ProjectDropdown", button_size))
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
							if (ImGui::Button(ctx_.lang->get("project.add_existing").c_str(), button_size))
							{
								utils::dialog_filter filter{ "VideoTagger Project", project_info::extension };
								auto result = utils::filesystem::get_file({}, { filter });

								if (result)
								{
									auto it = std::find_if(projects_.begin(), projects_.end(), [result](const project_info& project)
									{
										return std::filesystem::absolute(project.path) == std::filesystem::absolute(result.path);
									});

									if (it == projects_.end())
									{
										projects_.push_back(project_info::load_from_file(result.path));
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
							widgets::end_button_dropdown();
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
			temp_project = project_info{};
			path_from_name = true;
			ImGui::OpenPopup("Project Configuration", ImGuiPopupFlags_NoOpenOverExistingPopup);
		}
	}
}
