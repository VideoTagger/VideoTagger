#include <pch.hpp>
#include "main_window.hpp"
#include "app_context.hpp"
#include <fmt/format.h>

#include <widgets/widgets.hpp>
#include <widgets/tag_manager.hpp>
#include <widgets/video_widget.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/theme_customizer.hpp>
#include <widgets/time_input.hpp>
#include <widgets/inspector.hpp>
#include <widgets/modal/options.hpp>
#include <widgets/icons.hpp>
#include <widgets/controls.hpp>
#include <widgets/modal/keybind_popup.hpp>
#include <widgets/modal/keybind_options_popup.hpp>
#include <widgets/insert_segment_popup.hpp>

#include <utils/filesystem.hpp>

namespace vt
{
	main_window::main_window(const app_window_config& cfg) : app_window{ cfg }
	{
		ctx_.project_selector.on_click_project = [&](project_info& project_info)
		{
			debug::log("Clicked project: {}, Filepath: {}", project_info.name, project_info.path.u8string());
			if (!std::filesystem::is_regular_file(project_info.path))
			{
				const SDL_MessageBoxButtonData buttons[] = {
					// flags, buttonid, text
					{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
					{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Remove" },
					{ 0, 2, "Locate" },
				};

				SDL_MessageBoxData data{};
				data.flags = SDL_MESSAGEBOX_INFORMATION;

				//TODO: Replace title
				data.buttons = buttons;
				data.numbuttons = sizeof(buttons) / sizeof(buttons[0]);
				data.title = "VideoTagger";
				data.message = "This project no longer exists";
				int buttonid{};
				SDL_ShowMessageBox(&data, &buttonid);

				switch (buttonid)
				{
					case 1:
					{
						ctx_.project_selector.remove(project_info);
						ctx_.project_selector.on_project_list_update();
					}
					break;
					case 2:
					{
						utils::dialog_filter filter{ "VideoTagger Project", project::extension };
						auto result = utils::filesystem::get_file({}, { filter });
						if (result)
						{
							project_info = project_info::load_from_file(result.path);
							ctx_.project_selector.on_project_list_update();
						}
					}
					break;
				}
				return;
			}
			ctx_.current_project = project::load_from_file(project_info.path);
			ctx_.main_window->set_subtitle(ctx_.current_project->name);
		};

		ctx_.project_selector.on_project_list_update = [&]()
		{
			ctx_.project_selector.sort();
			ctx_.project_selector.save_projects_file(ctx_.projects_list_filepath);
			debug::log("Saving projects list to {}", std::filesystem::relative(ctx_.projects_list_filepath).u8string());
		};

		ctx_.group_browser.on_open_video = [this](video_id_t id)
		{
			auto* vinfo = ctx_.current_project->videos.get(id);
			vinfo->is_widget_open = true;
			ctx_.reset_player_docking = true;
			ctx_.current_project->videos.open_video(id);
		};
		init_options();
		load_settings();
		init_keybinds();
		init_player();
		fetch_themes();

		ctx_.project_selector.load_projects_file(ctx_.projects_list_filepath);
	}

	void main_window::on_close_project(bool should_shutdown)
	{
		//Save window size & state
		{
			auto& json_window = ctx_.settings["window"];
			if (ctx_.win_cfg.state == window_state::normal)
			{
				auto& size_setting = json_window["size"];
				int size[2] = {};
				SDL_GetWindowSize(window, &size[0], &size[1]);
				size_setting["width"] = size[0];
				size_setting["height"] = size[1];
			}
			json_window["state"] = ctx_.win_cfg.state;
			debug::log("Window size changing, saving settings file...");
			save_settings();
		}

		if (ctx_.current_project.has_value() and ctx_.is_project_dirty)
		{
			const SDL_MessageBoxButtonData buttons[] = {
				// flags, buttonid, text
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
				{ 0, 2, "Don't Save" },
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Save" },
			};

			SDL_MessageBoxData data{};
			data.flags = SDL_MESSAGEBOX_WARNING;

			//TODO: Replace title
			data.buttons = buttons;
			data.numbuttons = sizeof(buttons) / sizeof(buttons[0]);
			data.title = "VideoTagger";
			data.message = "The current project has unsaved changes.\nDo you want to save pending changes?";
			int buttonid{};
			SDL_ShowMessageBox(&data, &buttonid);

			switch (buttonid)
			{
				case 1:
				{
					on_save();
					if (should_shutdown) ctx_.state_ = app_state::shutdown;
				}
				break;
				case 2:
				{
					if (should_shutdown) ctx_.state_ = app_state::shutdown;
				}
				break;
			}
			return;
		}
		if (should_shutdown) ctx_.state_ = app_state::shutdown;
	}

	void main_window::on_save()
	{
		if (!ctx_.current_project.has_value()) return;
		debug::log("Saving project...");
		save_project();
	}

	void main_window::on_save_as()
	{
		if (!ctx_.current_project.has_value()) return;
		utils::dialog_filters filters{ utils::dialog_filter{ "VideoTagger Project", project::extension } };
		auto result = utils::filesystem::save_file({}, filters, ctx_.current_project->name);
		if (result)
		{
			debug::log("Saving project as {}", result.path.u8string());
			save_project_as(result.path);
		}
	}
	
	void main_window::on_show_in_explorer()
	{
		auto path = std::filesystem::absolute(ctx_.current_project->path.parent_path()).u8string();
		if (!path.empty())
		{
			utils::filesystem::open_in_explorer(path);
		}
	}

	//TODO: Shouldn't this be a static array somewhere or at least constexpr?
	static std::vector<std::string> valid_video_extensions()
	{
		return { "mp4", "mkv", "avi", "mov", "flv", "wmv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "vob", "mts", "m2ts", "mxf", "f4v", "divx", "rmvb", "asf", "swf" };
	}

	void main_window::on_import_videos()
	{
		if (!ctx_.current_project.has_value()) return;

		static std::vector<std::string> vid_exts = valid_video_extensions();

		static utils::dialog_filters filters
		{
			{ "Video", utils::filesystem::concat_extensions(vid_exts) },
		};

		auto result = utils::filesystem::get_files({}, filters);
		if (result)
		{
			for (const auto& path : result.paths)
			{
				{
					auto it = std::find_if(vid_exts.begin(), vid_exts.end(), [&path](const std::string& ext)
					{
						return path.extension() == "." + ext;
					});

					if (it == vid_exts.end())
					{
						//TODO: Should probably display a popup
						debug::error("Failed to import file {} - its not a valid video type", path.u8string());
						continue;
					}
				}

				const auto& videos = ctx_.current_project->videos;
				{
					auto it = std::find_if(videos.begin(), videos.end(), [path](const video_pool::iterator::value_type& video_data)
					{
						return video_data.second.path == path;
					});

					if (it == videos.end())
					{
						debug::log("Importing video {}", path.u8string());
						ctx_.current_project->video_import_tasks.push_back(ctx_.current_project->import_video(path));
					}
					else
					{
						//TODO: Display message box
					}
				}
			}
		}
	}

	void main_window::on_delete()
	{
		//TODO: This should be implemented as a function in timeline
		//also segments dont get deselected when windows other than Inspector are active, which should probably be changed
		if (!ctx_.video_timeline.selected_segment.has_value()) return;

		ctx_.is_project_dirty = true;
		auto it = ctx_.video_timeline.selected_segment->segments->find(ctx_.video_timeline.selected_segment->segment_it->start);
		if (it != ctx_.video_timeline.selected_segment->segments->end())
		{
			ctx_.video_timeline.selected_segment->segments->erase(it);
			ctx_.video_timeline.selected_segment.reset();
		}
	}

	bool main_window::load_settings()
	{
		float font_size = 18.0f;
		bool result = std::filesystem::exists(ctx_.app_settings_filepath);
		if (result)
		{
			//TODO: Error checking
			debug::log("Loading settings from: {}", ctx_.app_settings_filepath.string());
			ctx_.settings = utils::json::load_from_file(ctx_.app_settings_filepath);

			if (ctx_.settings.contains("window") and ctx_.settings["window"].contains("size"))
			{
				auto& size = ctx_.settings["window"]["size"];
				if (size.contains("width") and size.contains("height"))
				{
					SDL_SetWindowSize(window, size["width"].get<int>(), size["height"].get<int>());
					SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
				}
			}
			if (ctx_.settings.contains("thumbnail-size"))
			{
				ctx_.app_settings.thumbnail_size = ctx_.settings["thumbnail-size"];
			}
			if (ctx_.settings.contains("window") and ctx_.settings["window"].contains("state"))
			{
				auto& json_window = ctx_.settings["window"];
				auto state = json_window["state"].get<window_state>();
				switch (state)
				{
					case window_state::maximized: SDL_MaximizeWindow(window); break;
					default: break;
				}
				ctx_.win_cfg.state = state;

				if (json_window.contains("font-size"))
				{
					font_size = json_window["font-size"].get<float>();
				}
			}
			if (ctx_.settings.contains("first-launch"))
			{
				ctx_.first_launch = ctx_.settings["first-launch"];
				if (ctx_.first_launch)
				{
					ctx_.reset_layout = true;
				}
			}
			if (ctx_.settings.contains("show-windows"))
			{
				auto& show_windows = ctx_.settings["show-windows"];
				if (show_windows.contains("inspector")) ctx_.win_cfg.show_inspector_window = show_windows["inspector"];
				if (show_windows.contains("tag-manager")) ctx_.win_cfg.show_tag_manager_window = show_windows["tag-manager"];
				if (show_windows.contains("timeline")) ctx_.win_cfg.show_timeline_window = show_windows["timeline"];
				if (show_windows.contains("video-player")) ctx_.win_cfg.show_video_player_window = show_windows["video-player"];
				if (show_windows.contains("video-browser")) ctx_.win_cfg.show_video_browser_window = show_windows["video-browser"];
				if (show_windows.contains("video-group-browser")) ctx_.win_cfg.show_video_group_browser_window = show_windows["video-group-browser"];
				if (show_windows.contains("video-group-queue")) ctx_.win_cfg.show_video_group_queue_window = show_windows["video-group-queue"];
			}
			if (ctx_.settings.contains("load-thumbnails"))
			{
				ctx_.app_settings.load_thumbnails = ctx_.settings.at("load-thumbnails");
			}
		}
		else
		{
			ctx_.reset_layout = true;
		}

		auto& io = ImGui::GetIO();
		if (!std::filesystem::exists(io.IniFilename))
		{
			ctx_.reset_layout = true;
		}
		ctx_.settings["first-launch"] = false;
		build_fonts(font_size);
		return result;
	}

	void main_window::save_settings()
	{
		if (!ctx_.app_settings_filepath.empty())
		{
			debug::log("Saving app settings...");
			utils::json::write_to_file(ctx_.settings, ctx_.app_settings_filepath);
		}
		else
		{
			debug::error("Settings filepath is empty");
		}
	}

	void main_window::save_project()
	{
		if (!ctx_.current_project.has_value()) return;

		ctx_.current_project->save();
		ctx_.is_project_dirty = false;
	}

	void main_window::save_project_as(const std::filesystem::path& filepath)
	{
		if (!ctx_.current_project.has_value()) return;

		ctx_.current_project->save_as(filepath);
		ctx_.is_project_dirty = false;
	}

	void main_window::close_project()
	{
		on_close_project(false);
		ctx_.reset_current_video_group();
		ctx_.current_project = std::nullopt;
		ctx_.video_timeline.selected_segment = std::nullopt;
		set_subtitle();
	}

	void main_window::init_keybinds()
	{
		ctx_.keybinds.clear();

		keybind_flags flags(true, false, false);
		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::save_project), keybind(SDLK_s, keybind_modifiers{ true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save();
		})));

		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::save_project_as), keybind(SDLK_s, keybind_modifiers{ true, true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save_as();
		})));

		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::show_in_explorer), keybind(SDLK_o, keybind_modifiers{ true, false, true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_show_in_explorer();
		})));

		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::import_videos), keybind(SDLK_i, keybind_modifiers{ true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_import_videos();
		})));

		ctx_.keybinds.insert("Delete", keybind(SDLK_DELETE, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_delete();
		})));

		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::close_project), keybind(SDLK_F4, keybind_modifiers{ true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			close_project();
		})));

		ctx_.keybinds.insert(ctx_.lang.get(lang_pack_id::exit), keybind(SDLK_F4, keybind_modifiers{ false, false, true }, flags,
			builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_close_project(true);
		})));

		keybind_modifiers toggle_window_mod{ false, true };
		ctx_.keybinds.insert("Toggle Video Player", keybind(SDLK_F1, toggle_window_mod, flags, toggle_window_action("video-player", ctx_.win_cfg.show_video_player_window)));
		ctx_.keybinds.insert("Toggle Video Browser", keybind(SDLK_F2, toggle_window_mod, flags, toggle_window_action("video-browser", ctx_.win_cfg.show_video_browser_window)));
		ctx_.keybinds.insert("Toggle Video Group Browser", keybind(SDLK_F3, toggle_window_mod, flags, toggle_window_action("video-group-browser", ctx_.win_cfg.show_video_group_browser_window)));
		ctx_.keybinds.insert("Toggle Video Group Queue", keybind(SDLK_F4, toggle_window_mod, flags, toggle_window_action("video-group-queue", ctx_.win_cfg.show_video_group_queue_window)));
		ctx_.keybinds.insert("Toggle Inspector", keybind(SDLK_F5, toggle_window_mod, flags, toggle_window_action("inspector", ctx_.win_cfg.show_inspector_window)));
		ctx_.keybinds.insert("Toggle Tag Manager", keybind(SDLK_F6, toggle_window_mod, flags, toggle_window_action("tag-manager", ctx_.win_cfg.show_tag_manager_window)));
		ctx_.keybinds.insert("Toggle Timeline", keybind(SDLK_F7, toggle_window_mod, flags, toggle_window_action("timeline", ctx_.win_cfg.show_timeline_window)));

		keybind_modifiers player_mod{};
		ctx_.keybinds.insert("Play/Pause", keybind(SDLK_SPACE, player_mod, flags, player_action(player_action_type::play_pause)));

		keybind_modifiers player_secondary_mod{ false, true };
		ctx_.keybinds.insert("Seek Forwards", keybind(SDLK_RIGHT, player_mod, flags, player_action(player_action_type::forwards)));
		ctx_.keybinds.insert("Seek Backwards", keybind(SDLK_LEFT, player_mod, flags, player_action(player_action_type::backwards)));
		ctx_.keybinds.insert("Skip To Next", keybind(SDLK_RIGHT, player_secondary_mod, flags, player_action(player_action_type::skip_next)));
		ctx_.keybinds.insert("Skip To Previous", keybind(SDLK_LEFT, player_secondary_mod, flags, player_action(player_action_type::skip_previous)));
		ctx_.keybinds.insert("Toggle Looping", keybind(SDLK_l, keybind_modifiers{ true }, flags, player_action(player_action_type::toggle_looping)));
	}

	void main_window::init_options()
	{
		auto& options = ctx_.options;
		static auto display_keybinds_panel = [&](keybind_storage& keybinds, bool toggleable = true, bool show_actions = true, bool can_add_new = true)
		{
			static auto validator = [&keybinds](const std::string& name, const vt::keybind& kb, keybind_validator_mode mode) -> bool
			{
				if (mode == keybind_validator_mode::validate_keybind_and_name) return keybinds.is_valid(name, kb);
				switch (mode)
				{
					case keybind_validator_mode::validate_keybind:
					{
						auto it = std::find_if(keybinds.begin(), keybinds.end(), [&](const std::pair<std::string, vt::keybind>& kb_)
						{
							return kb_.second == kb;
						});
						return it == keybinds.end() and kb.key_code >= 0;
					}
					case keybind_validator_mode::validate_name: return !keybinds.contains(name);
				}
				debug::panic("Unreachable validator code path");
				return false;
			};

			if (can_add_new)
			{
				auto& io = ImGui::GetIO();
				auto avail = ImGui::GetContentRegionAvail();
				if (ImGui::Button("Add Keybind", { avail.x, ImGui::GetTextLineHeightWithSpacing() * 1.5f * io.FontGlobalScale }))
				{
					ImGui::OpenPopup("##KeybindCreationPopup");
				}
				static std::vector<std::shared_ptr<keybind_action>> actions;
				static std::string keybind_name;
				static int selected_action{};


				if (widgets::modal::keybind_options_popup("##KeybindCreationPopup", keybind_name, input::last_keybind, actions, selected_action, keybind_options_config::show_name_field | keybind_options_config::show_keybind_field | keybind_options_config::show_action_field | keybind_options_config::creation_mode, validator, keybind_validator_mode::validate_keybind_and_name))
				{
					keybind_flags flags(true, true, true);
					auto kb = keybind(input::last_keybind.key_code, input::last_keybind.modifiers, flags, input::last_keybind.action);

					keybinds.insert(keybind_name, kb);
					ctx_.is_project_dirty = true;
					input::last_keybind.action = nullptr;
				}
				ImGui::Separator();
			}

			if (keybinds.empty())
			{
				auto avail_area = ImGui::GetContentRegionAvail();
				constexpr const char* text = "No keybinds to display...";
				auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
				auto cpos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(avail_area / 2 - half_text_size);
				ImGui::BeginDisabled();
				ImGui::TextWrapped(text);
				ImGui::EndDisabled();
				ImGui::SetCursorPos(cpos);
				return;
			}

			std::optional<std::string> rename_kb_name;
			std::optional<std::string> delete_kb_name;
			static std::string new_kb_name;
			static std::vector<std::shared_ptr<keybind_action>> actions;

			if (ImGui::BeginTable("##ApplicationKeybinds", 2 + (int)toggleable + (int)show_actions, ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit, ImGui::GetContentRegionAvail()))
			{
				if (toggleable)
				{
					ImGui::TableSetupColumn(nullptr);
				}
				ImGui::TableSetupColumn("Shortcut Name");
				ImGui::TableSetupColumn("Keybind");
				if (show_actions)
				{
					ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed);
				}
				ImGui::BeginDisabled();
				ImGui::TableHeadersRow();
				ImGui::EndDisabled();
				int row{};
				auto& style = ImGui::GetStyle();
				auto& io = ImGui::GetIO();

				for (auto& [name, keybind] : keybinds)
				{
					ImGui::TableNextRow();
					const char* id = name.c_str();
					bool is_row_selected = ImGui::TableGetHoveredRow() - 1 == row;
					if (toggleable)
					{
						ImGui::TableNextColumn();
						bool enabled = keybind.flags.enabled;
						if (widgets::checkbox(("##KeybindEnabled" + name).c_str(), &enabled))
						{
							keybind.flags.enabled = enabled;
							ctx_.is_project_dirty = true;
						}

						ImGui::SameLine();
						ImGui::PushID(id);
						if (!is_row_selected) ImGui::BeginDisabled();
						std::string delete_kb_id = fmt::format("{}##DeleteKb{}", icons::delete_, name);
						if (widgets::icon_button(delete_kb_id.c_str()))
						{
							delete_kb_name = name;
						}
						if (!is_row_selected) ImGui::EndDisabled();
						ImGui::PopID();
					}
					ImGui::TableNextColumn();
					ImGui::Text(id);

					//TODO: This is repeated 3 times, refactor this
					{
						ImGui::PushID(id);
						std::string edit_id = std::string(icons::edit) + "##KbName";
						is_row_selected = ImGui::TableGetHoveredRow() - 1 == row or (ImGui::GetHoveredID() == ImGui::GetID(edit_id.c_str()));
						ImGui::SameLine();
						if (keybind.flags.rebindable and is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
						{
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { style.FramePadding.x * 0.5f, style.FramePadding.y });
							if (widgets::icon_button(edit_id.c_str()))
							{
								new_kb_name = name;
								actions = get_all_keybind_actions();
								ImGui::OpenPopup("##KeybindNamePopup");
							}
							ImGui::PopStyleVar();
						}
						else
						{
							ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
							ImGui::Dummy(style.ItemSpacing);
						}

						int dummy{};
						if (widgets::modal::keybind_options_popup("##KeybindNamePopup", new_kb_name, keybind, actions, dummy, keybind_options_config::show_name_field | keybind_options_config::show_save_button, validator, keybind_validator_mode::validate_name))
						{
							rename_kb_name = name;
						}
						ImGui::PopID();
					}

					ImGui::TableNextColumn();
					std::string key_combination = keybind.name(false);
					ImGui::Text(key_combination.c_str());

					if (keybind.flags.rebindable)
					{
						ImGui::PushID(id);
						std::string edit_combination_id = fmt::format("{}{}", icons::edit, "##EditCombination");
						is_row_selected |= (ImGui::GetHoveredID() == ImGui::GetID(edit_combination_id.c_str()));

						ImGui::SameLine();
						if (is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
						{
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { style.FramePadding.x * 0.5f, style.FramePadding.y });
							if (widgets::icon_button(edit_combination_id.c_str()))
							{
								//resets the last keybind
								input::last_keybind.key_code = -1;
								ImGui::OpenPopup("##KeybindPopup");
							}
							ImGui::PopStyleVar();
						}
						else
						{
							ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
							ImGui::Dummy(style.ItemSpacing);
						}

						if (widgets::modal::keybind_popup("##KeybindPopup", keybind, input::last_keybind, validator))
						{
							keybind.rebind(input::last_keybind);
							ctx_.is_project_dirty = true;
						}
						ImGui::PopID();
					}

					if (show_actions)
					{
						ImGui::TableNextColumn();
						ImGui::Text(keybind.action->name().c_str());

						if (keybind.flags.rebindable)
						{
							static std::vector<std::shared_ptr<keybind_action>> actions;

							ImGui::PushID(id);
							std::string edit_kb_id = fmt::format("{}##EditKb", icons::edit);
							is_row_selected = ImGui::TableGetHoveredRow() - 1 == row or (ImGui::GetHoveredID() == ImGui::GetID(edit_kb_id.c_str()));
							ImGui::SameLine();
							if (is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { style.FramePadding.x * 0.5f, style.FramePadding.y });
								if (widgets::icon_button(edit_kb_id.c_str()))
								{
									actions = get_all_keybind_actions();
									ImGui::OpenPopup("##KeybindOptionsPopup");
								}
								ImGui::PopStyleVar();
							}
							else
							{
								ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
								ImGui::Dummy(style.ItemSpacing);
							}

							auto it = std::find_if(actions.begin(), actions.end(), [&](const std::shared_ptr<keybind_action>& action)
							{
								return keybind.action->name() == action->name();
							});

							int selected_action = (it != actions.end() ? static_cast<int>(std::distance(actions.begin(), it)) : 0);
							std::string dummy;
							//FIXME: Putting 'keybind' applies the changes immediately and it should be applied only after save
							if (widgets::modal::keybind_options_popup("##KeybindOptionsPopup", dummy, keybind, actions, selected_action, keybind_options_config::show_action_field | keybind_options_config::show_save_button, validator, keybind_validator_mode::validate_keybind))
							{
								ctx_.is_project_dirty = true;
							}
							ImGui::PopID();
						}
					}
					++row;
				}
				ImGui::EndTable();
			}

			if (rename_kb_name.has_value())
			{
				keybinds.rename(rename_kb_name.value(), new_kb_name);
				ctx_.is_project_dirty = true;
			}

			if (delete_kb_name.has_value())
			{
				keybinds.erase(delete_kb_name.value());
				ctx_.is_project_dirty = true;
			}
		};

		options("Application Settings", "General") = [this]()
		{
			widgets::label("Font Size");
			ImGui::SameLine();
			int start_font_size = static_cast<int>(ctx_.fonts["default"]->FontSize);
			static int font_size = start_font_size;
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("000").x);
			//TODO: Add messagebox informing that the changes will be applied only after restart
			if (ImGui::DragInt("##FontSize", &font_size, 1.0f, 8, 72, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				ctx_.settings["window"]["font-size"] = font_size;
			}

			float text_height = ImGui::GetTextLineHeight();

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Thumbnail Size");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 4);
			if (ImGui::DragFloat("##ThumbnailSizeDrag", &ctx_.app_settings.thumbnail_size, 0.5f, 45.0f, 100.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
			{
				ctx_.settings["thumbnail-size"] = ctx_.app_settings.thumbnail_size;
			}

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Load Thumbnails");
			ImGui::SameLine();
			if (ImGui::Checkbox("##LoadThumbnailsCheckbox", &ctx_.app_settings.load_thumbnails))
			{
				ctx_.settings["load-thumbnails"] = ctx_.app_settings.load_thumbnails;
			}

			//TODO: Add theme selection

#ifdef _DEBUG
			auto& io = ImGui::GetIO();
			ImGui::SeparatorText("Debug Only");
			ImGui::DragFloat("Font Scale", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
#endif
		};
		options("Application Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.keybinds, false, false, false);
		};

		options("Project Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.current_project->keybinds);
		};
		options.set_active_tab("Application Settings", "General");
	}

	void main_window::init_player()
	{
		ctx_.player.callbacks.on_set_playing = [](bool is_playing)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_playing(is_playing);
			//}
			if (ctx_.current_video_group_id() == invalid_video_group_id)
			{
				return;
			}
			ctx_.displayed_videos.set_playing(is_playing);
		};

		ctx_.player.callbacks.on_set_looping = [](loop_mode mode)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_looping(is_looping);
			//}
			if (ctx_.current_video_group_id() == invalid_video_group_id)
			{
				return;
			}
		};

		ctx_.player.callbacks.on_set_speed = [](float speed)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_speed(speed);
			//}

			if (ctx_.current_video_group_id() == invalid_video_group_id)
			{
				return;
			}
			ctx_.displayed_videos.set_speed(speed);
		};

		ctx_.player.callbacks.on_skip = [](int dir, loop_mode mode, bool is_playing)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;

			video_group_playlist::iterator it;
			if (dir > 0)
			{
				it = playlist.next();
			}
			else if (dir < 0)
			{
				it = playlist.previous();
			}

			ctx_.reset_current_video_group();
			if (mode == loop_mode::all and it == playlist.end())
			{
				it = playlist.set_current(playlist.begin());
			}

			if (it != playlist.end())
			{
				ctx_.set_current_video_group_id(*it);
				ctx_.displayed_videos.set_playing(is_playing);
			}
		};

		ctx_.player.callbacks.on_seek = [](std::chrono::nanoseconds ts)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.seek(ts);
			//}

			if (ctx_.current_video_group_id() == invalid_video_group_id)
			{
				return;
			}
			ctx_.displayed_videos.seek(ts);
		};

		ctx_.player.callbacks.on_finish = [](loop_mode mode, bool is_playing)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;

			if (mode == loop_mode::one)
			{
				ctx_.displayed_videos.seek(std::chrono::nanoseconds{ 0 });
				ctx_.displayed_videos.set_playing(true);
				return;
			}

			if (ctx_.app_settings.next_video_on_end)
			{
				ctx_.player.reset_data();

				auto it = playlist.next();
				ctx_.reset_current_video_group();
				if (mode == loop_mode::all and it == playlist.end())
				{
					it = playlist.set_current(playlist.begin());
				}

				if (it != playlist.end())
				{
					ctx_.set_current_video_group_id(*it);
				}

				ctx_.displayed_videos.set_playing(true);
			}
		};
	}

	void main_window::fetch_themes()
	{
		ctx_.themes.clear();
		if (!std::filesystem::is_directory(ctx_.theme_dir_filepath)) return;
		for (auto& dir_entry : std::filesystem::directory_iterator(ctx_.theme_dir_filepath))
		{
			if (!dir_entry.is_regular_file()) continue;

			auto path = dir_entry.path();
			if (path.extension() != fmt::format(".{}", theme::extension)) continue;
			ctx_.themes.push_back(path);
		}
	}

	void main_window::draw_menubar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::file)))
			{
				std::string key_name = ctx_.lang.get(lang_pack_id::import_videos);
				auto& kb = ctx_.keybinds.at(key_name);
				std::string menu_name = fmt::format("{} {}", icons::import_, key_name);
				if (ImGui::MenuItem(menu_name.c_str(), kb.name().c_str()))
				{
					on_import_videos();
				}
				ImGui::Separator();
				{
					std::string key_name = ctx_.lang.get(lang_pack_id::save_project);
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = fmt::format("{} {}", icons::save, key_name);
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_save();
					}
				}

				{

					std::string key_name = ctx_.lang.get(lang_pack_id::save_project_as);
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = fmt::format("{} {}", icons::save_as, key_name);
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_save_as();
					}
				}
				ImGui::Separator();
				{
					{
						std::string key_name = ctx_.lang.get(lang_pack_id::show_in_explorer);
						auto& kb = ctx_.keybinds.at(key_name);
						std::string menu_name = fmt::format("{} {}", icons::folder, ctx_.lang.get(lang_pack_id::show_in_explorer));
						if (ImGui::MenuItem(menu_name.c_str(), kb.name().c_str()))
						{
							on_show_in_explorer();
						}
					}

					{
						std::string menu_name = fmt::format("{} {}", icons::import_export, ctx_.lang.get(lang_pack_id::import_export));
						if (ImGui::BeginMenu(menu_name.c_str()))
						{
							ImGui::MenuItem("Import Tags", nullptr, &ctx_.win_cfg.show_tag_importer_window, true);

							ImGui::Separator();
							if (ImGui::MenuItem("Export Tags"))
							{
								utils::dialog_filter filter{ "VideoTagger Tags", "vttags" };
								auto result = utils::filesystem::save_file({}, { filter }, ctx_.current_project->name);
								if (result)
								{
									nlohmann::ordered_json json;
									json["version"] = ctx_.current_project->version;
									json["tags"] = ctx_.current_project->tags;
									utils::json::write_to_file(json, result.path);
								}
							}
							if (ImGui::MenuItem("Export Segments", nullptr, nullptr, ctx_.current_video_group_id() != invalid_video_group_id))
							{
								const auto& group_name = ctx_.current_project->video_groups.at(ctx_.current_video_group_id()).display_name;

								utils::dialog_filter filter{ "VideoTagger Segments", "vtss" };
								auto result = utils::filesystem::save_file({}, { filter }, group_name);
								if (result)
								{
									//TODO: ability to choose which groups to export

									ctx_.current_project->export_segments(result.path, { ctx_.current_video_group_id() });

								}
							}
							ImGui::EndMenu();
						}
					}
				}
				ImGui::Separator();
				{
					std::string key_name = "Close Project";
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = std::string(icons::close) + ' ' + ctx_.lang.get(lang_pack_id::close_project);
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						close_project();
					}
				}
				ImGui::Separator();
				{
					std::string key_name = "Exit";
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = std::string(icons::exit) + ' ' + ctx_.lang.get(lang_pack_id::exit);
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_close_project(true);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::edit)))
			{
				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::undo), nullptr, nullptr, false))
				{

				}

				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::redo), nullptr, nullptr, false))
				{

				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::window)))
			{
				auto& windows = ctx_.settings["show-windows"];

				std::string key_name = "Save Project";
				auto& kb = ctx_.keybinds.at(key_name);
				std::string menu_item = fmt::format("{} {}", icons::save, key_name);

				struct win_toggles
				{
					const char* name{};
					const char* keybind_name{};
					const char* settings_name{};
					bool* value{};
				};

				//TODO: This could be done in a better way
				for (auto& [name, keybind_name, settings_name, value] :
					{
						win_toggles{ "Show Video Player", "Toggle Video Player", "video-player", &ctx_.win_cfg.show_video_player_window },
						win_toggles{ "Show Video Browser", "Toggle Video Browser", "video-browser", &ctx_.win_cfg.show_video_browser_window },
						win_toggles{ "Show Video Group Browser", "Toggle Video Group Browser", "video-group-browser", &ctx_.win_cfg.show_video_group_browser_window },
						win_toggles{ "Show Video Group Queue", "Toggle Video Group Queue", "video-group-queue", &ctx_.win_cfg.show_video_group_queue_window },
						win_toggles{},
						win_toggles{ "Show Inspector", "Toggle Inspector", "inspector", &ctx_.win_cfg.show_inspector_window },
						win_toggles{ "Show Tag Manager", "Toggle Tag Manager", "tag-manager", &ctx_.win_cfg.show_tag_manager_window },
						win_toggles{ "Show Timeline", "Toggle Timeline", "timeline", &ctx_.win_cfg.show_timeline_window },

					})
				{
					if (name == nullptr)
					{
						ImGui::Separator();
						continue;
					}

					auto& kb = ctx_.keybinds.at(keybind_name);
					std::string shortcut = kb.name();
					if (ImGui::MenuItem(name, shortcut.c_str(), value))
					{
						windows[settings_name] = *value;
					}
				}
#ifdef _DEBUG
				ImGui::SeparatorText("Debug Only");
				ImGui::MenuItem("Show Options", nullptr, &ctx_.win_cfg.show_options_window);
				ImGui::MenuItem("Show Theme Customizer", nullptr, &ctx_.win_cfg.show_theme_customizer_window);
#endif

				ImGui::Separator();
				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::redock_videos)))
				{
					ctx_.reset_player_docking = true;
				}
				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::reset_layout)))
				{
					ctx_.reset_layout = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::run)))
			{
				std::function<bool(const std::filesystem::path&)> has_scripts = [&has_scripts](const std::filesystem::path& path)
				{
					if (!std::filesystem::is_directory(path)) return false;
					for (const auto& dir_entry : std::filesystem::directory_iterator(path))
					{
						if (dir_entry.is_directory() and has_scripts(dir_entry))
						{
							return true;
						}
						else if (dir_entry.is_regular_file() and dir_entry.path().extension() == ".py")
						{
							return true;
						}
					}
					return false;
				};

				auto menu_name = fmt::format("{} {}", icons::folder_code, "Scripts");
				if (ImGui::BeginMenu(menu_name.c_str(), has_scripts(ctx_.scripts_filepath)))
				{
					std::function<void(const std::filesystem::path&)> draw_folder = [&draw_folder, &has_scripts](const std::filesystem::path& path)
					{
						for (const auto& dir_entry : std::filesystem::directory_iterator(path))
						{
							auto entry_path = dir_entry.path();
							if (dir_entry.is_regular_file() and entry_path.extension() == ".py")
							{
								std::string script_name = entry_path.stem().string();
								std::string script_menu_name = fmt::format("{} {}", icons::terminal, script_name);
								if (ImGui::MenuItem(script_menu_name.c_str()))
								{
									ctx_.script_eng.run(script_name, "on_run");
								}
							}
							else if (dir_entry.is_directory() and !std::filesystem::is_empty(dir_entry))
							{
								auto dir_name = fmt::format("{} {}", icons::folder_code, entry_path.stem().string());
								if (has_scripts(entry_path) and ImGui::BeginMenu(dir_name.c_str()))
								{
									draw_folder(entry_path);
									ImGui::EndMenu();
								}
							}
						}
					};

					draw_folder(ctx_.scripts_filepath);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::tools)))
			{
				if (ImGui::BeginMenu("Themes"))
				{
					if (ImGui::MenuItem("Reload"))
					{
						fetch_themes();
					}
					ImGui::Separator();

					//TODO: Select current theme
					for (const auto& path : ctx_.themes)
					{
						auto name = path.stem().u8string();
						if (ImGui::MenuItem(name.c_str()))
						{
							auto theme = theme::load_from_file(path);
							ImGui::GetStyle() = theme.style;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::MenuItem("Theme Customizer", nullptr, &ctx_.win_cfg.show_theme_customizer_window);
				ImGui::Separator();
				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::options)))
				{
					ctx_.win_cfg.show_options_window = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(ctx_.lang.get(lang_pack_id::help)))
			{
				if (ImGui::MenuItem(ctx_.lang.get(lang_pack_id::about)))
				{
					ctx_.win_cfg.show_about_window = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ctx_.win_cfg.show_about_window)
		{
			ImGui::OpenPopup("AboutPopup");
		}
		if (ctx_.win_cfg.show_options_window)
		{
			ImGui::OpenPopup("Options");
		}
		if (ctx_.win_cfg.show_tag_importer_window)
		{
			ctx_.tag_importer.open();
		}

		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
			auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.2f, ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("AboutPopup", &ctx_.win_cfg.show_about_window, flags))
			{
				const auto& style = ImGui::GetStyle();

				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::Text("About VideoTagger");
				ImGui::Separator();
				ImGui::Dummy(style.ItemSpacing);
				ImGui::PopFont();

				ImGui::TextDisabled("Version: %s", "1.0.0.0");
				ImGui::Dummy(style.ItemSpacing);

				auto button_size = ImVec2{ ImGui::GetContentRegionAvail().x, 0 };
				if (ImGui::Button("Close", button_size) or (!ImGui::IsWindowHovered() and ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
				{
					ctx_.win_cfg.show_about_window = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();
		}
	}

	static void handle_insert_segment()
	{
		//TODO: check if start and end have value

		static std::optional<std::string> insert_key;

		for (auto it = ctx_.insert_segment_data.begin(); it != ctx_.insert_segment_data.end() and !insert_key.has_value();)
		{
			auto& insert_data = it->second;

			if (!insert_data.tag.empty() and insert_data.name_index < 0)
			{
				auto& tags = ctx_.video_timeline.displayed_tags();

				auto it = std::find(tags.begin(), tags.end(), insert_data.tag);
				if (it != tags.end())
				{
					insert_data.name_index = static_cast<int>(it - tags.begin());
				}
			}

			if (insert_data.show_insert_popup)
			{
				ImGui::OpenPopup("###AppInsertSegment");
				insert_key = it->first;
				break;
			}

			if (!insert_data.ready)
			{
				it++;
				continue;
			}

			auto& segments = ctx_.get_current_segment_storage().at(insert_data.tag);

			if (insert_data.show_merge_popup)
			{
				auto overlapping = segments.find_range(*insert_data.start, *insert_data.end);
				if (!overlapping.empty())
				{
					ImGui::OpenPopup("##MergePopupApp");
					insert_key = it->first;
					break;
				}
			}

			segments.insert(*insert_data.start, *insert_data.end);
			it = ctx_.insert_segment_data.erase(it);
			insert_key.reset();
		}

		if (!insert_key.has_value())
		{
			return;
		}

		auto insert_data_it = ctx_.insert_segment_data.find(*insert_key);
		if (insert_data_it == ctx_.insert_segment_data.end())
		{
			insert_key.reset();
			ctx_.insert_segment_data.erase(insert_data_it);
			return;
		}

		auto& insert_data = insert_data_it->second;

		auto min_ts = ctx_.video_timeline.start_timestamp().seconds_total.count();
		auto max_ts = ctx_.video_timeline.end_timestamp().seconds_total.count();
		//should it be this or all tags?
		auto& tags = ctx_.video_timeline.displayed_tags();

		auto segment_type = *insert_data.start == *insert_data.end ? tag_segment_type::timestamp : tag_segment_type::segment;
		const char* insert_segment_popup_id = segment_type == tag_segment_type::timestamp ? "Insert Timestamp###AppInsertSegment" : "Insert Segment###AppInsertSegment";

		static int selected_tag_index{};

		bool presed_ok{};
		if (widgets::insert_segment_popup(insert_segment_popup_id, *insert_data.start, *insert_data.end, segment_type, min_ts, max_ts, tags, insert_data.name_index, presed_ok))
		{
			if (presed_ok)
			{
				insert_data.tag = tags.at(insert_data.name_index);
				insert_data.show_insert_popup = false;
				insert_data.ready = true;
			}
			else
			{
				ctx_.insert_segment_data.erase(insert_data_it);
			}

			insert_key.reset();
			return;
		}

		bool pressed_ok{};
		if (widgets::merge_segments_popup("##MergePopupApp", presed_ok, false))
		{
			if (presed_ok)
			{
				insert_data.show_merge_popup = false;
			}
			else
			{
				ctx_.insert_segment_data.erase(insert_data_it);
			}

			insert_key.reset();
			return;
		}
	}

	void main_window::draw_main_app()
	{
		ImGuiWindowClass window_class{};
		window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge | ImGuiViewportFlags_TopMost;
		ImGui::SetNextWindowClass(&window_class);

		auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		auto win_open = ImGui::BeginPopupModal("Script Progress", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::PopStyleVar(2);

		if (win_open)
		{
			ImGuiWindow* win = ImGui::GetCurrentWindow();
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			auto wind = (SDL_Window*)ImGui::GetCurrentWindow()->Viewport->PlatformHandle;

			auto width = ImGui::GetContentRegionAvail().x;

			static uint8_t dot_count = 0;
			static constexpr uint8_t max_dots = 3;
			static float elapsed_acc{};

			auto elapsed_time = ImGui::GetIO().DeltaTime;

			if (elapsed_acc >= 0.75)
			{
				elapsed_acc = 0;
				dot_count = (dot_count + 1) % (max_dots + 1);
			}
			elapsed_acc += elapsed_time;

			std::string suffix = std::string(dot_count, '.');
			ImGui::Text("Script Progress%s", suffix.c_str());
			ImGui::ProgressBar(0.5f, ImVec2{ width, ImGui::GetTextLineHeight() / 2.5f }, "");
			ImGui::EndPopup();
		}

		if (!ctx_.current_project.has_value()) return;
		draw_menubar();
		if (!ctx_.current_project.has_value()) return;

		{
			static bool was_popup_opened = false;
			static bool resume_video = false;
			if (ctx_.pause_player)
			{
				if (!was_popup_opened)
				{
					was_popup_opened = true;
					resume_video = ctx_.displayed_videos.is_playing();
					ctx_.displayed_videos.set_playing(false);
				}
			}
			else
			{
				if (was_popup_opened)
				{
					was_popup_opened = false;
					ctx_.displayed_videos.set_playing(resume_video);
					resume_video = false;
				}
			}

			ctx_.pause_player = false;
		}

		{
			auto& tasks = ctx_.current_project->video_import_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto status = it->wait_for(std::chrono::nanoseconds(0));
				if (status == std::future_status::ready)
				{
					auto result = it->get();
					if (!result.success)
					{
						debug::error("Failed to import {}", result.video_path.u8string());
					}
					else if (ctx_.app_settings.load_thumbnails)
					{
						auto video_data = ctx_.current_project->videos.get(result.video_id);
						video_data->update_thumbnail();
					}

					it = tasks.erase(it);
					break;
					//continue;
				}

				++it;
			}
		}

		handle_insert_segment();

		//TODO: probably should be done somewhere else
		ctx_.update_current_video_group();

		if (ctx_.win_cfg.show_video_player_window)
		{
			video_player_data data = ctx_.player.data();
			//TODO: include offsets
			//auto& vids = ctx_.current_project->videos;
			//if (vids.size() > 0)
			//{
			//	std::vector<std::chrono::nanoseconds> durations;
			//	for (const auto& [id, vinfo] : vids)
			//	{
			//		if (!vinfo.is_widget_open) continue;
			//		durations.push_back(vinfo.video.duration());
			//	}
			//
			//	auto min_it = std::min_element(durations.begin(), durations.end());
			//	if (min_it != durations.end())
			//	{
			//		data.end_ts = *min_it;
			//		ctx_.player.update_data(data);
			//	}
			//}
			static bool reset_player_when_group_is_invalid = false;
			if (ctx_.current_video_group_id() == invalid_video_group_id)
			{
				if (reset_player_when_group_is_invalid)
				{
					data.current_ts = std::chrono::nanoseconds{ 0 };
					data.start_ts = std::chrono::nanoseconds{ 0 };
					data.end_ts = std::chrono::nanoseconds{ 0 };
					ctx_.player.update_data(data, false);

					reset_player_when_group_is_invalid = false;
				}
			}
			else
			{
				//TODO: probably could be done only when needed instead of on every frame.
				// Video timeline does the same thing and group duration needs to be calculated
				data.current_ts = ctx_.displayed_videos.current_timestamp();
				data.start_ts = std::chrono::nanoseconds{ 0 };
				data.end_ts = ctx_.displayed_videos.duration();
				ctx_.player.update_data(data, ctx_.displayed_videos.is_playing());

				reset_player_when_group_is_invalid = true;
			}

			ctx_.player.render();
		}

		/*uint64_t vid_id{};
		if (ctx_.player.is_visible())
		{
			for (auto& [id, vinfo] : ctx_.current_project->videos)
			{
				if (!vinfo.is_widget_open) continue;
				auto& vid = vinfo.video;

				widgets::draw_video_widget(vid, vinfo.is_widget_open, vid_id++);
			}
		}*/

		//TODO: This breaks when there are undocked videos
		if (ctx_.player.is_visible())
		{
			uint64_t vid_id{};
			for (auto& video_data : ctx_.displayed_videos)
			{
				auto pool_data = ctx_.current_project->videos.get(video_data.id);
				if (!pool_data->is_widget_open) continue;

				bool timestamp_in_range = video_data.is_timestamp_in_range(ctx_.displayed_videos.current_timestamp());

				widgets::draw_video_widget(*video_data.video, video_data.display_texture, timestamp_in_range, pool_data->is_widget_open, vid_id++);
			}
		}

		if (ctx_.reset_player_docking)
		{
			ctx_.player.dock_windows(4);
			ctx_.reset_player_docking = false;
		}

		if (ctx_.win_cfg.show_timeline_window/* and ctx_.current_video_group_id != 0*/)
		{
			auto group_duration = ctx_.displayed_videos.duration();

			//TODO: Definitely change this!
			ctx_.video_timeline.set_video_group_id(ctx_.current_video_group_id());
			ctx_.video_timeline.set_tag_storage(&ctx_.current_project->tags);
			ctx_.video_timeline.set_segment_storage(ctx_.current_video_group_id() != invalid_video_group_id ? &ctx_.get_current_segment_storage() : nullptr);
			ctx_.video_timeline.sync_tags();
			ctx_.video_timeline.set_start_timestamp(timestamp::zero());
			ctx_.video_timeline.set_end_timestamp(timestamp(std::chrono::duration_cast<std::chrono::seconds>(group_duration)));
			ctx_.video_timeline.set_current_timestamp(timestamp{ std::chrono::duration_cast<std::chrono::seconds>(ctx_.displayed_videos.current_timestamp()) });
			ctx_.video_timeline.insert_segment_container = &ctx_.insert_segment_data;

			ctx_.video_timeline.render(ctx_.win_cfg.show_timeline_window);

			if (ctx_.video_timeline.current_timestamp().seconds_total != std::chrono::duration_cast<std::chrono::seconds>(ctx_.displayed_videos.current_timestamp()))
			{
				ctx_.displayed_videos.seek(ctx_.video_timeline.current_timestamp().seconds_total);
			}
		}

		if (ctx_.win_cfg.show_tag_manager_window)
		{
			static std::optional<widgets::tag_rename_data> tag_rename;
			static std::optional<widgets::tag_delete_data> tag_delete;
			widgets::draw_tag_manager_widget(ctx_.current_project->tags, tag_rename, tag_delete, ctx_.is_project_dirty, ctx_.win_cfg.show_tag_manager_window);

			//TODO: Should this be done in the widget or outside?
			if (tag_rename.has_value() and tag_rename->ready)
			{
				//TODO: maybe make this a function in the project class

				ctx_.is_project_dirty = true;

				for (auto& [_, displayed_tags] : ctx_.video_timeline.displayed_tags_per_group())
				{
					for (auto& tag_name : displayed_tags)
					{
						if (tag_name == tag_rename->old_name)
						{
							tag_name = tag_rename->new_name;
						}
					}
				}

				for (auto& [group_id, segments] : ctx_.current_project->segments)
				{
					auto node_handle = segments.extract(tag_rename->old_name);
					if (!node_handle.empty())
					{
						node_handle.key() = tag_rename->new_name;
						segments.insert(std::move(node_handle));
					}
				}

				//TODO: consider renaming tags in keybinds

				tag_rename.reset();
			}
			if (tag_delete.has_value() and tag_delete->ready)
			{
				ctx_.is_project_dirty = true;

				auto& selected_segment = ctx_.video_timeline.selected_segment;
				if (selected_segment.has_value() and selected_segment->tag->name == tag_delete->tag)
				{
					selected_segment.reset();
				}

				auto& moving_segment = ctx_.video_timeline.moving_segment;
				if (moving_segment.has_value() and moving_segment->tag->name == tag_delete->tag)
				{
					moving_segment.reset();
				}

				auto& segments = ctx_.current_project->segments;

				for (auto it = segments.begin(); it != segments.end(); ++it)
				{
					auto& group_segments = it->second;
					auto group_segments_it = group_segments.find(tag_delete->tag);
					if (group_segments_it != group_segments.end())
					{
						group_segments.erase(group_segments_it);
					}
				}

				ctx_.current_project->tags.erase(tag_delete->tag);

				tag_delete.reset();
			}
		}

		//TODO: Add base virtual class that has render(bool&) method instead of this

		ctx_.tag_importer.render(ctx_.win_cfg.show_tag_importer_window);

		if (ctx_.win_cfg.show_options_window)
		{
			ctx_.options.render(&ctx_.win_cfg.show_options_window);
		}

		if (ctx_.win_cfg.show_inspector_window)
		{
			widgets::inspector(ctx_.video_timeline.selected_segment, ctx_.video_timeline.moving_segment, ctx_.app_settings.link_start_end_segment, ctx_.is_project_dirty, &ctx_.win_cfg.show_inspector_window);
		}

		if (ctx_.win_cfg.show_video_browser_window)
		{
			ctx_.browser.render(ctx_.win_cfg.show_video_browser_window);
		}

		if (ctx_.win_cfg.show_video_group_browser_window)
		{
			ctx_.group_browser.render(ctx_.win_cfg.show_video_group_browser_window);
		}

		if (ctx_.win_cfg.show_video_group_queue_window)
		{
			ctx_.group_queue.current_group_id = ctx_.current_video_group_id();
			ctx_.group_queue.render(ctx_.win_cfg.show_video_group_queue_window);
		}

		if (ctx_.win_cfg.show_theme_customizer_window)
		{
			ctx_.theme_customizer.render(ctx_.win_cfg.show_theme_customizer_window);
		}

		//ImGui::ShowDemoWindow();
		//ImGui::OpenPopup("Script Progress");
	}

	void main_window::draw_project_selector()
	{
		if (!ctx_.current_project.has_value())
		{
			ctx_.project_selector.render();
			ctx_.project_selector.set_opened(true);
			return;
		}
	}

	void main_window::draw()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);
		ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

		constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //| ImGuiDockNodeFlags_NoDocking
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove; //| ImGuiWindowFlags_NoDocking
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		ImGui::Begin("##Editor", NULL, window_flags);
		ImGui::PopStyleVar(3);

		if (ctx_.reset_layout and ImGui::DockBuilderGetNode(dockspace_id) != nullptr)
		{
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			auto dockspace_id_copy = dockspace_id;
			auto main_dock_right = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id_copy);
			auto main_dock_up = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Up, 0.7f, nullptr, &dockspace_id_copy);
			auto main_dock_up_left = ImGui::DockBuilderSplitNode(main_dock_up, ImGuiDir_Left, 0.25f, nullptr, &main_dock_up);
			auto main_dock_down = ImGui::DockBuilderSplitNode(main_dock_up, ImGuiDir_Down, 0.25f, nullptr, &main_dock_up);
			auto dock_right_up = ImGui::DockBuilderSplitNode(main_dock_right, ImGuiDir_Up, 0.5f, nullptr, &main_dock_right);
			ImGui::DockBuilderDockWindow("Inspector", dock_right_up);
			ImGui::DockBuilderDockWindow("Tag Manager", main_dock_right);
			ImGui::DockBuilderDockWindow("Group Queue", main_dock_down);
			ImGui::DockBuilderDockWindow("Video Player", main_dock_up);
			ImGui::DockBuilderDockWindow("Video Browser", main_dock_up_left);
			ImGui::DockBuilderDockWindow("Theme Customizer", main_dock_up);
			ImGui::DockBuilderDockWindow("Timeline", dockspace_id_copy);
			ImGui::DockBuilderDockWindow("Video Group Browser", dockspace_id_copy);
			/*for (size_t i = 0; i < 4; ++i)
			{
				auto video_id = "Video##" + std::to_string(i);
				ImGui::DockBuilderDockWindow(video_id.c_str(), main_dock_up);
			}*/

			ImGui::DockBuilderFinish(dockspace_id);
			ctx_.reset_layout = false;
		}

		ImGui::DockSpace(dockspace_id, ImVec2{}, dockspace_flags);
		ctx_.current_project.has_value() ? draw_main_app() : draw_project_selector();

		ImGui::End();
	}

	void main_window::handle_event(const SDL_Event& event)
	{
		app_window::handle_event(event);

		decltype(ctx_.current_project->keybinds)* project_keybinds{};
		if (ctx_.current_project.has_value())
		{
			project_keybinds = &ctx_.current_project->keybinds;
		}
		input::process_event(event, ctx_.keybinds, project_keybinds);

		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				if (event.window.windowID != SDL_GetWindowID(window)) return;

				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_MINIMIZED:
					{
						ctx_.win_cfg.state = window_state::minimized;
					}
					break;
					case SDL_WINDOWEVENT_MAXIMIZED:
					{
						ctx_.win_cfg.state = window_state::maximized;
					}
					break;
					case SDL_WINDOWEVENT_RESTORED:
					{
						ctx_.win_cfg.state = window_state::normal;
					}
					break;
					case SDL_WINDOWEVENT_CLOSE:
					{
						on_close_project(true);
					}
					break;
				}
			}
			break;
		}
	}
}