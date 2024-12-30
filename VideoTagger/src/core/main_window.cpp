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
#include <embeds/about.hpp>

#include <utils/filesystem.hpp>
#include <editor/run_script_command.hpp>
#include <editor/selected_attribute_query.hpp>
#include <ImGuizmo.h>
#include <utils/matrix.hpp>
#include <utils/vec.hpp>
#include <utils/intersection.hpp>
#include <editor/set_selected_attribute_command.hpp>
#include <utils/string.hpp>

extern "C"
{
	#include <libavutil/ffversion.h>
}

#include <openssl/opensslv.h>
#include <pybind11/pybind11.h>

namespace vt
{
	static void show_debug_info()
	{
		SDL_version compiled;
		SDL_version linked;
		SDL_VERSION(&compiled);
		SDL_GetVersion(&linked);
		debug::log("SDL Version (Header):  {}.{}.{}", compiled.major, compiled.minor, compiled.patch);
		debug::log("SDL Version (Linked):  {}.{}.{}", linked.major, linked.minor, linked.patch);
		debug::log("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
		debug::log("ImGui Version: {}", IMGUI_VERSION);
		debug::log("FFmpeg Version: {}", FFMPEG_VERSION);
		debug::log("OpenSSL Version: {}", OPENSSL_FULL_VERSION_STR);
		debug::log("Python Version: {}", PY_VERSION);
		debug::log("pybind11 Version: {}.{}.{}", PYBIND11_VERSION_MAJOR, PYBIND11_VERSION_MINOR, PYBIND11_VERSION_PATCH);
	}

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
			ctx_.console.clear();
		};

		ctx_.project_selector.on_project_list_update = [&]()
		{
			ctx_.project_selector.sort();
			ctx_.project_selector.save_projects_file(ctx_.projects_list_filepath);
			debug::log("Saving projects list to {}", std::filesystem::relative(ctx_.projects_list_filepath).u8string());
		};

		ctx_.group_browser.on_open_video = [this](video_id_t id)
		{
			//auto& vid_resource = ctx_.current_project->videos.get(id);
			ctx_.reset_player_docking = true;
			//ctx_.current_project->videos.open_video(id);
		};
		show_debug_info();

		init_options();
		load_settings();
		init_keybinds();
		init_player();
		fetch_themes();
		load_accounts();
		ctx_.scripts = fetch_scripts(ctx_.script_dir_filepath);

		ctx_.project_selector.load_projects_file(ctx_.projects_list_filepath);
	}

	bool main_window::on_close_project(bool should_shutdown)
	{
		if (ctx_.script_handle.has_value())
		{
			ctx_.script_eng.interrupt();
			return false;
		}

		if (ctx_.current_project.has_value())
		{
			for (auto& download_task : ctx_.current_project->video_download_tasks)
			{
				download_task.task.cancel();
			}
		}

		ctx_.gizmo_target = nullptr;
		ctx_.last_focused_video = std::nullopt;
		ctx_.registry.execute<set_selected_attribute_command>(nullptr);

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
				}
				break;
				case 0: return false;
			}
		}
		if (should_shutdown) ctx_.state_ = app_state::shutdown;
		return true;
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

	void main_window::on_import_videos()
	{
		//TODO: implement
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

	bool main_window::load_accounts()
	{
		if (!std::filesystem::exists(ctx_.accounts_filepath))
		{
			return false;
		}

		auto accounts_json = utils::json::load_from_file(ctx_.accounts_filepath);
		for (auto& [service_id, service_accounts] : accounts_json.items())
		{
			if (ctx_.account_managers.count(service_id) == 0)
			{
				debug::log("Accounts file contains unsupported service: {}", service_id);
				continue;
			}

			auto& manager = ctx_.account_managers.at(service_id);
			manager->load(service_accounts);
		}

		return true;
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
				if (show_windows.contains("shape-attributes")) ctx_.win_cfg.show_inspector_window = show_windows["shape-attributes"];
				if (show_windows.contains("tag-manager")) ctx_.win_cfg.show_tag_manager_window = show_windows["tag-manager"];
				if (show_windows.contains("timeline")) ctx_.win_cfg.show_timeline_window = show_windows["timeline"];
				if (show_windows.contains("video-player")) ctx_.win_cfg.show_video_player_window = show_windows["video-player"];
				if (show_windows.contains("video-browser")) ctx_.win_cfg.show_video_browser_window = show_windows["video-browser"];
				if (show_windows.contains("video-group-browser")) ctx_.win_cfg.show_video_group_browser_window = show_windows["video-group-browser"];
				if (show_windows.contains("video-group-queue")) ctx_.win_cfg.show_video_group_queue_window = show_windows["video-group-queue"];
				if (show_windows.contains("console")) ctx_.win_cfg.show_console_window = show_windows["console"];
			}
			if (ctx_.settings.contains("load-thumbnails"))
			{
				ctx_.app_settings.load_thumbnails = ctx_.settings.at("load-thumbnails");
			}
			if (ctx_.settings.contains("autoplay"))
			{
				ctx_.app_settings.autoplay = ctx_.settings.at("autoplay");
			}
			if (ctx_.settings.contains("clear-console-on-run"))
			{
				ctx_.app_settings.clear_console_on_run = ctx_.settings.at("clear-console-on-run");
			}
			if (ctx_.settings.contains("enable-undocking"))
			{
				ctx_.app_settings.enable_undocking = ctx_.settings.at("enable-undocking");
			}
			if (ctx_.settings.contains("enable-gizmo-scaling"))
			{
				ctx_.app_settings.enable_gizmo_scaling = ctx_.settings.at("enable-gizmo-scaling");
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
		if (on_close_project(false))
		{
			ctx_.reset_current_video_group();
			ctx_.current_project = std::nullopt;
			ctx_.video_timeline.selected_segment = std::nullopt;
			ctx_.is_project_dirty = false;
			set_subtitle();
		}
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
		ctx_.keybinds.insert("Toggle Shape Attributes", keybind(SDLK_F6, toggle_window_mod, flags, toggle_window_action("shape-attributes", ctx_.win_cfg.show_shape_attributes_window)));
		ctx_.keybinds.insert("Toggle Tag Manager", keybind(SDLK_F7, toggle_window_mod, flags, toggle_window_action("tag-manager", ctx_.win_cfg.show_tag_manager_window)));
		ctx_.keybinds.insert("Toggle Timeline", keybind(SDLK_F8, toggle_window_mod, flags, toggle_window_action("timeline", ctx_.win_cfg.show_timeline_window)));
		ctx_.keybinds.insert("Toggle Console", keybind(SDLK_F9, toggle_window_mod, flags, toggle_window_action("console", ctx_.win_cfg.show_console_window)));

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
					ImGui::TextUnformatted(id);

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
					ImGui::TextUnformatted(key_combination.c_str());

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
						ImGui::TextUnformatted(keybind.action->name().c_str());

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
			ImGui::SeparatorText("UI");
			ImGui::TextUnformatted("Scale Gizmos");
			ImGui::SameLine();
			if (ImGui::Checkbox("##GizmoScalingCheckbox", &ctx_.app_settings.enable_gizmo_scaling))
			{
				ctx_.settings["enable-gizmo-scaling"] = ctx_.app_settings.enable_gizmo_scaling;
			}
		};
		options("Application Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.keybinds, false, false, false);
		};

		options("Project Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.current_project->keybinds);
		};

		options("Storage Settings", "Accounts") = []()
		{
			//TODO: maybe make this a widget
			
			//TODO: move this somewhere so it can have a value per account
			static bool login_in_progress = false;
			bool modifed_account = false;

			static std::map<std::string, std::future<bool>> retry_login_futures;

			for (auto& [service_id, account_manager] : ctx_.account_managers)
			{
				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::TextUnformatted(account_manager->service_display_name().c_str());
				ImGui::PopFont();

				auto account_status = account_manager->login_status();
				if (!login_in_progress and (account_status == account_login_status::logged_in or account_status == account_login_status::refresh_failed))
				{
					std::string user_name = fmt::format("User: {}", account_manager->account_name());
					ImGui::TextUnformatted(user_name.c_str());

					if (account_status == account_login_status::logged_in)
					{
						ImGui::TextColored(ImVec4{ 0.0f, 0.9f, 0.0f, 1.0f }, "Logged in");

						if (ImGui::Button("Remove account"))
						{
							account_manager->log_out();
							modifed_account = true;
						}
					}
					else
					{
						if (retry_login_futures.count(service_id))
						{
							ImGui::TextUnformatted("Retrying...");

							auto& future = retry_login_futures.at(service_id);
							if (future.wait_for(std::chrono::seconds{}) == std::future_status::ready)
							{
								retry_login_futures.erase(service_id);
							}
						}
						else
						{
							ImGui::AlignTextToFramePadding();
							ImGui::TextColored(ImVec4{ 0.9f, 0.0f, 0.0f, 1.0f }, "Login failed");

							ImGui::SameLine();
							if (widgets::icon_button(icons::retry))
							{
								retry_login_futures[service_id] = account_manager->retry_login();
								modifed_account = true;
							}


							if (ImGui::Button("Remove account"))
							{
								account_manager->log_out();
								modifed_account = true;
							}
						}
					}
				}
				else
				{
					std::string popup_id = fmt::format("Log in to a {} account", account_manager->service_display_name());
					if (ImGui::BeginPopupModal(popup_id.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
					{
						bool success{};
						if (account_manager->draw_login_popup(success))
						{
							if (success)
							{
								modifed_account = true;
							}

							debug::log("Login popup success: {}", success);
							ImGui::CloseCurrentPopup();
							login_in_progress = false;
						}
						ImGui::EndPopup();
					}
					if (ImGui::Button("Log in"))
					{
						ImGui::OpenPopup(popup_id.c_str());
						login_in_progress = true;
					}
				}

				if (modifed_account)
				{
					nlohmann::ordered_json accounts_json;
					for (auto& [service_id, manager] : ctx_.account_managers)
					{
						accounts_json[service_id] = *manager;
					}

					utils::json::write_to_file(accounts_json, ctx_.accounts_filepath);
				}
			}
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

			if (ctx_.app_settings.autoplay)
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

	utils::file_node main_window::fetch_scripts(const std::filesystem::path& path)
	{
		utils::file_node result;
		if (std::filesystem::exists(path))
		{
			for (const auto& dir_entry : std::filesystem::directory_iterator(path))
			{
				auto entry_path = dir_entry.path();
				if (dir_entry.is_regular_file() and utils::string::to_lowercase(entry_path.extension().string()) == ".py")
				{
					auto script_path = std::filesystem::relative(entry_path, ctx_.script_dir_filepath);
					result.insert(script_path);
				}
				else if (dir_entry.is_directory() and !std::filesystem::is_empty(dir_entry))
				{
					auto key = std::filesystem::relative(entry_path, ctx_.script_dir_filepath);
					result[key] = fetch_scripts(dir_entry.path());
				}
			}
		}
		return result;
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
				if (ImGui::BeginMenu(menu_name.c_str()))
				{
					for (auto& [importer_id, importer] : ctx_.video_importers)
					{
						if (!importer->available())
						{
							continue;
						}

						std::string menu_importer_name = fmt::format("{} {}", importer->importer_display_icon(), importer->importer_display_name());
						if (ImGui::MenuItem(menu_importer_name.c_str()))
						{
							ctx_.current_project->prepare_video_import(importer_id);
						}
					}

					ImGui::EndMenu();
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
							if (ImGui::MenuItem("Import Tags", nullptr, &ctx_.win_cfg.show_tag_importer_window))
							{
								utils::dialog_filters filters{ { "VideoTagger Tags", "vttags" } };
								auto result = utils::filesystem::get_file({}, filters);
								if (result)
								{
									ctx_.tag_importer.tags_path = result.path;
								}
								else
								{
									ctx_.win_cfg.show_tag_importer_window = false;
								}

							}

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
							ImGui::Separator();
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
						win_toggles{ "Show Shape Attributes", "Toggle Shape Attributes", "shape-attributes", &ctx_.win_cfg.show_shape_attributes_window },
						win_toggles{ "Show Tag Manager", "Toggle Tag Manager", "tag-manager", &ctx_.win_cfg.show_tag_manager_window },
						win_toggles{ "Show Timeline", "Toggle Timeline", "timeline", &ctx_.win_cfg.show_timeline_window },
						win_toggles{ "Show Console", "Toggle Console", "console", &ctx_.win_cfg.show_console_window},

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
				if (ImGui::MenuItem("Enable Undocking", nullptr, ctx_.app_settings.enable_undocking))
				{
					ctx_.app_settings.enable_undocking = !ctx_.app_settings.enable_undocking;
					ctx_.settings["enable-undocking"] = ctx_.app_settings.enable_undocking;
					save_settings();
					enable_undocking(ctx_.app_settings.enable_undocking);
				}
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
						else if (dir_entry.is_regular_file() and utils::string::to_lowercase(dir_entry.path().extension().string()) == ".py")
						{
							return true;
						}
					}
					return false;
				};

				auto menu_name = fmt::format("{} {}", icons::folder_code, "Scripts");
				if (ImGui::IsWindowAppearing())
				{
					ctx_.scripts = fetch_scripts(ctx_.script_dir_filepath);
				}

				if (ImGui::BeginMenu(menu_name.c_str(), !ctx_.scripts.empty()))
				{
					std::function<void(const utils::file_node&)> draw_folder = [&draw_folder, &has_scripts](const utils::file_node& node)
					{
						for (const auto& [path, folder] : node)
						{
							auto dir_name = fmt::format("{} {}", icons::folder_code, path.stem().string());
							if (!folder.empty() and ImGui::BeginMenu(dir_name.c_str()))
							{
								draw_folder(folder);
								ImGui::EndMenu();
							}
						}

						if (!node.folders.empty())
						{
							ImGui::Separator();
						}

						for (auto& child : node.children)
						{
							std::string script_path = child.stem().string();
							std::string script_menu_name = fmt::format("{} {}", icons::terminal, script_path);
							if (ImGui::MenuItem(script_menu_name.c_str()))
							{
								ctx_.registry.execute<run_script_command>(child);
							}
						}
					};

					draw_folder(ctx_.scripts);
					ImGui::EndMenu();
				}

				menu_name = fmt::format("{} {}", icons::folder, "Open Scripts Folder");
				if (ImGui::MenuItem(menu_name.c_str(), nullptr, nullptr, std::filesystem::exists(ctx_.script_dir_filepath)))
				{
					utils::filesystem::open_in_explorer(std::filesystem::absolute(ctx_.script_dir_filepath));
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
		if (ctx_.win_cfg.show_tag_importer_window and !ctx_.tag_importer.is_open())
		{
			ctx_.tag_importer.open();
		}

		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
			auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
			ImVec2 window_size = ImGui::GetContentRegionMax() * ImVec2 { 0.333f, 0.4f };
			ImGui::SetNextWindowSize(window_size, ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("AboutPopup", &ctx_.win_cfg.show_about_window, flags))
			{
				const auto& style = ImGui::GetStyle();

				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::TextUnformatted("About VideoTagger");
				ImGui::Separator();
				ImGui::Dummy(style.ItemSpacing);
				ImGui::PopFont();

				ImGui::BeginDisabled();

				ImGui::Text("Version: %s", "1.0.0.0");
				ImGui::Dummy(style.ItemSpacing);

				ImGui::TextWrapped("%s", embed::app_description);

#ifdef _DEBUG
				ImGui::SeparatorText("Debug Only");

				SDL_version compiled;
				SDL_version linked;
				SDL_VERSION(&compiled);
				SDL_GetVersion(&linked);
				ImGui::Text("SDL Version (Header):  %u.%u.%u", compiled.major, compiled.minor, compiled.patch);
				ImGui::Text("SDL Version (Linked):  %u.%u.%u", linked.major, linked.minor, linked.patch);
				ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
				ImGui::Text("ImGui Version: %s", IMGUI_VERSION);
				ImGui::Text("FFmpeg Version: %s", FFMPEG_VERSION);
				ImGui::Text("OpenSSL Version: %s", OPENSSL_FULL_VERSION_STR);
				ImGui::Text("Python Version: %s", PY_VERSION);
				ImGui::Text("pybind11 Version: %u.%u.%u", PYBIND11_VERSION_MAJOR, PYBIND11_VERSION_MINOR, PYBIND11_VERSION_PATCH);
#endif

				ImGui::EndDisabled();

				ImGui::Separator();
				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::TextUnformatted("Third Party Libraries");
				ImGui::PopFont();

				ImVec2 child_size = ImGui::GetContentRegionAvail();
				child_size.y -= ImGui::GetTextLineHeightWithSpacing() + style.WindowPadding.y;

				ImGuiChildFlags child_flags = ImGuiChildFlags_None;

				if (ImGui::BeginChild("##ThirdPartyArea", child_size, child_flags))
				{
					for (const auto& [name, license] : embed::third_party_licenses)
					{
						if (widgets::begin_collapsible(fmt::format("##{}", name), name, 0, icons::license))
						{
							ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_WindowBg]);
							if (ImGui::BeginTable("##Background", 1, ImGuiTableFlags_RowBg))
							{
								ImGui::TableNextColumn();

								ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
								ImGui::TextWrapped("%s", license.c_str());
								ImGui::PopStyleColor();

								ImGui::EndTable();
							}
							ImGui::PopStyleColor();
							widgets::end_collapsible();
						}
					}
				}
				ImGui::EndChild();

				auto button_size = ImVec2{ ImGui::GetContentRegionAvail().x, 0 };
				if (ImGui::Button("Close", button_size) or ImGui::IsKeyReleased(ImGuiKey_Escape))
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
				auto& tags = ctx_.current_project->displayed_tags;

				if (auto it = ctx_.current_project->find_displayed_tag(insert_data.tag); it != tags.end())
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

		auto min_ts = ctx_.video_timeline.start_timestamp().total_milliseconds.count();
		auto max_ts = ctx_.video_timeline.end_timestamp().total_milliseconds.count();
		//should it be this or all tags?
		auto& tags = ctx_.current_project->displayed_tags;

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
			auto& tasks = ctx_.current_project->prepare_video_import_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				if (!task())
				{
					++it;
					continue;
				}

				for (auto& import_data : task.import_data)
				{
					ctx_.current_project->schedule_video_import(task.importer_id, std::move(import_data), utils::uuid::get());
				}
				it = tasks.erase(it);

				//TODO: set some frame time limit;
				break;
			}
		}

		{
			auto& tasks = ctx_.current_project->video_import_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				auto vid_resource = task();
				if (vid_resource != nullptr)
				{
					video_id_t video_id = vid_resource->id();
					if (ctx_.current_project->import_video(std::move(vid_resource), task.group_id))
					{
						if (ctx_.app_settings.load_thumbnails)
						{
							ctx_.current_project->schedule_generate_thumbnail(video_id);
						}
					}
				}
				it = tasks.erase(it);
				//TODO: set some frame time limit;
				break;
			}
		}

		{
			auto& tasks = ctx_.current_project->generate_thumbnail_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				if (!task())
				{
					debug::error("Failed to generate thumbnail");
				}

				it = tasks.erase(it);
				//TODO: set some frame time limit;
				break;
			}
		}

		{
			auto& tasks = ctx_.current_project->video_download_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				if (!task.task.is_done())
				{
					++it;
					continue;
				}

				std::string video_name = "NAME_UNKNOWN";
				if (ctx_.current_project->videos.contains(task.video_id))
				{
					video_name = ctx_.current_project->videos.get(task.video_id).metadata().title.value_or(video_name);
				}

				auto status = task.task.result.get();
				if (status == video_download_status::failure)
				{
					debug::error("Failed to download video {} ({})", video_name, task.video_id);
					ctx_.console.add_entry(widgets::console::entry::flag_type::error, fmt::format("Failed to download video {} ({})", video_name, task.video_id));
				}
				else
				{
					debug::log("Downloaded video {} ({})", video_name, task.video_id);
					dynamic_cast<downloadable_video_resource&>(ctx_.current_project->videos.get(task.video_id)).set_file_path(task.task.data->download_path.u8string());
					ctx_.console.add_entry(widgets::console::entry::flag_type::info, fmt::format("Downloaded video {} ({})", video_name, task.video_id));
				}

				it = tasks.erase(it);
			}
		}

		{
			auto& tasks = ctx_.current_project->video_refresh_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				if (task.task.wait_for(std::chrono::seconds{}) != std::future_status::ready)
				{
					++it;
					continue;
				}

				it = tasks.erase(it);
			}
		}

		{
			auto& tasks = ctx_.current_project->remove_video_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				task.task.get();

				it = tasks.erase(it);
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
			tag_attribute_instance* selected_attribute = ctx_.registry.execute_query<selected_attribute_query>();
			bool has_selected_attribute = selected_attribute != nullptr;

			bool is_shape = has_selected_attribute and selected_attribute->has<shape>() and selected_attribute->get<shape>().get_type() != shape::type::none;
			if (ctx_.last_focused_video.has_value() and ctx_.displayed_videos.find(ctx_.last_focused_video.value()) == ctx_.displayed_videos.end())
			{
				ctx_.last_focused_video = std::nullopt;
				ctx_.registry.execute<set_selected_attribute_command>(nullptr);
				ctx_.gizmo_target = nullptr;
			}

			if (!is_shape)
			{
				ctx_.gizmo_target = nullptr;
			}

			uint64_t vid_id{};
			for (auto& video_data : ctx_.displayed_videos)
			{
				bool timestamp_in_range = video_data.is_timestamp_in_range(ctx_.displayed_videos.current_timestamp());
				auto selected_segment = ctx_.video_timeline.selected_segment;

				//TODO: handle is_widget_open
				bool is_widget_open = true;
				ImVec2 point_pos{};
				bool has_target = ctx_.gizmo_target != nullptr;
				if (has_target)
				{
					point_pos = { (float)ctx_.gizmo_target->at(0), (float)ctx_.gizmo_target->at(1) };
				}

				widgets::draw_video_widget(video_data.video, video_data.display_texture, timestamp_in_range, is_widget_open, vid_id++, [&point_pos, has_selected_attribute, selected_attribute, is_shape, has_target, &video_data](ImVec2 pos, ImVec2 size, ImVec2 tex_size)
				{
					static constexpr auto orange = tag_attribute::type_color(tag_attribute::type::shape); //0xFF30A0F0;
					static auto from_tex_pos = [&pos, &tex_size, &size](const ImVec2 point) -> ImVec2
					{
						return pos + (point / tex_size) * size;
					};

					static auto to_tex_pos = [&pos, &tex_size, &size](const ImVec2 point) -> utils::vec2<uint32_t>
					{
						ImVec2 tex_coords = (point - pos) / size * tex_size;
						return utils::vec2<uint32_t>{ static_cast<uint32_t>(std::round(tex_coords.x)), static_cast<uint32_t>(std::round(tex_coords.y)) };
					};

					static auto from_pixels = [&tex_size, &size](uint32_t value) -> float
					{
						float viewport_diagonal = utils::intersection::length(size);
						float tex_diagonal = utils::intersection::length(tex_size);
						return (float)value * viewport_diagonal / tex_diagonal;
					};

					static auto to_pixels = [&tex_size, &size](float value) -> uint32_t
					{
						float viewport_diagonal = utils::intersection::length(size);
						float tex_diagonal = utils::intersection::length(tex_size);
						return (uint32_t)(value * tex_diagonal / viewport_diagonal);
					};

					ImGuiIO& io = ImGui::GetIO();
					bool hovered = ImGui::IsWindowHovered();
					bool focused = ImGui::IsWindowFocused();
					auto border_color = hovered ? 0xFF00FF00 : 0xFF0000FF;
					float border_thickness = 2.0f;

					if (focused)
					{
						ctx_.last_focused_video = video_data.id;
					}

					bool last_focused = ctx_.last_focused_video == video_data.id;

					bool is_polygon = is_shape and selected_attribute->get<shape>().get_type() == shape::type::polygon;

					ImVec2 add_point_pos{};
					bool add_point{};

					auto current_ts = ctx_.video_timeline.current_timestamp();
					bool can_add_point{};

					bool is_keyframe{};
					if (is_shape)
					{
						const auto& shape = selected_attribute->get<vt::shape>();
						shape.visit([current_ts, &is_keyframe](const auto& map)
						{
							if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
							{
								auto it = map.find(current_ts);
								if (it != map.end())
								{
									is_keyframe = true;
								}
							}
						});
					}

					if (is_polygon)
					{
						const auto& shape = selected_attribute->get<vt::shape>();
						can_add_point = is_keyframe;
					}

					if (last_focused and is_shape and ImGui::BeginPopupContextItem("##VideoCtxMenu"))
					{
						auto& shape = selected_attribute->get<vt::shape>();
						bool close = false;
						const auto& style = ImGui::GetStyle();

						if (can_add_point and ImGui::MenuItem("Add Point", nullptr, nullptr))
						{
							add_point_pos = ImGui::GetWindowPos();
							add_point = true;
						}

						if (ImGui::MenuItem(fmt::format("{} Add Keyframe", icons::keyframe).c_str(), nullptr, nullptr, !is_keyframe))
						{
							shape.visit([current_ts, &is_keyframe, &shape](auto& map)
							{
								if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
								{
									auto it = map.lower_bound(current_ts);
									if (map.empty())
									{
										map[current_ts].push_back({});
									}
									else
									{
										if (it != map.begin() and (it == map.end() or it->first != current_ts))
										{
											--it;
										}
										map[current_ts] = it->second;
									}
									is_keyframe = true;
									ctx_.is_project_dirty = true;
								}
							});
						}

						if (ImGui::MenuItem(fmt::format("{} Add Region", shape.type_icon(shape.get_type())).c_str(), nullptr, nullptr, is_keyframe))
						{
							shape.visit([current_ts, &is_keyframe](auto& map)
							{
								if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
								{
									auto& keyframe = map.at(current_ts);
									keyframe.push_back({});
									keyframe.back().set_target(ctx_.gizmo_target);
									ctx_.is_project_dirty = true;
								}
							});
						}

						if (ImGui::BeginMenu("Transform", has_target))
						{
							auto icon_size = ImGui::CalcTextSize(icons::align_center).x;
							ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
							if (ImGui::BeginTable("##AlignTable", 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInner, { 3.f * (icon_size + 2 * style.FramePadding.x), 0.f }))
							{
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_horizontal_center))
								{
									point_pos.x = tex_size.x / 2;
									close = true;
								}
								widgets::tooltip("Align Horizontal Center");
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_vertical_top))
								{
									point_pos.y = 0;
									close = true;
								}
								widgets::tooltip("Align Vertical Top");

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_horizontal_left))
								{
									point_pos.x = 0;
									close = true;
								}
								widgets::tooltip("Align Horizontal Left");
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_center))
								{
									point_pos = tex_size / 2;
									close = true;
								}
								widgets::tooltip("Align Center");
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_horizontal_right))
								{
									point_pos.x = tex_size.x;
									close = true;
								}
								widgets::tooltip("Align Horizontal Right");

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_vertical_center))
								{
									point_pos.y = tex_size.y / 2;
									close = true;
								}
								widgets::tooltip("Align Vertical Center");
								ImGui::TableNextColumn();
								if (widgets::icon_button(icons::align_vertical_bottom))
								{
									point_pos.y = tex_size.y;
									close = true;
								}
								widgets::tooltip("Align Vertical Bottom");

								ImGui::EndTable();
							}
							ImGui::PopStyleVar();
							ImGui::EndMenu();
						}

						if (close)
						{
							ctx_.gizmo_target->at(0) = (uint32_t)point_pos.x;
							ctx_.gizmo_target->at(1) = (uint32_t)point_pos.y;
							ctx_.is_project_dirty = true;
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					if (!add_point and can_add_point and (ImGui::IsKeyDown(ImGuiKey_LeftShift) or ImGui::IsKeyDown(ImGuiKey_RightShift)) and ImGui::IsMouseClicked(0) and hovered)
					{
						add_point_pos = ImGui::GetMousePos();
						add_point = true;
					}
					else if (is_shape and !add_point and ImGui::IsMouseClicked(0) and hovered)
					{
						auto& shape = selected_attribute->get<vt::shape>();
						auto closest_target = shape.closest_point(current_ts, to_tex_pos(ImGui::GetMousePos()), from_pixels(10));
						if (closest_target != nullptr)
						{
							ctx_.gizmo_target = closest_target;
						}
					}

					if (add_point and can_add_point and is_polygon)
					{
						auto& shape = selected_attribute->get<vt::shape>();
						auto& map = shape.get_map<polygon>();
						auto& polygons = map.at(current_ts); //this keyframe definitely exists, it was checked before

						auto it = std::find_if(polygons.begin(), polygons.end(), [](const polygon& poly)
						{
							for (const auto& vertex : poly.vertices)
							{
								if (ctx_.gizmo_target == &vertex) return true;
							}
							return false;
						});

						bool all_empty = true;
						for (const auto& poly : polygons)
						{
							if (!poly.vertices.empty())
							{
								all_empty = false;
								break;
							}
						}

						if (all_empty)
						{
							polygons.front().vertices.push_back({ to_tex_pos(add_point_pos) });
						}
						else if (it == polygons.end())
						{
							//add new polygon with that point
							polygons.push_back(polygon{ { to_tex_pos(add_point_pos) } });
						}
						else
						{
							auto& polygon = *it;
							auto& pos = add_point_pos;

							auto closest_it = polygon.vertices.end();
							float min_distance = std::numeric_limits<float>::infinity();

							for (auto it = polygon.vertices.begin(); it != polygon.vertices.end(); ++it)
							{
								auto next_it = std::next(it);
								if (next_it == polygon.vertices.end())
								{
									next_it = polygon.vertices.begin();
								}

								const auto& vertex1 = *it;
								const auto& vertex2 = *next_it;

								float new_distance = utils::intersection::distance_to_segment(pos, from_tex_pos({ (float)vertex1[0], (float)vertex1[1] }), from_tex_pos({ (float)vertex2[0], (float)vertex2[1] }));


								if (new_distance < min_distance)
								{
									min_distance = new_distance;
									closest_it = it;
								}
							}

							if (closest_it != polygon.vertices.end())
							{
								auto next_it = std::next(closest_it);
								if (next_it == polygon.vertices.end())
								{
									next_it = polygon.vertices.begin();
								}

								ctx_.gizmo_target = &*polygon.vertices.insert(next_it, to_tex_pos(pos));
							}
							else
							{
								ctx_.gizmo_target = &*polygon.vertices.insert(closest_it, to_tex_pos(pos));
							}
						}
					}

					auto draw_list = ImGui::GetWindowDrawList();
					//draw_list->AddRectFilled(top_left, bottom_right, overlay_color);
					/*draw_list->AddLine(top_left, bottom_right, border_color, border_thickness);
					draw_list->AddLine(top_right, bottom_left, border_color, border_thickness);*/

					ImVec2 top_left = { pos.x, pos.y };
					ImVec2 top_right = { pos.x + size.x, pos.y };
					ImVec2 bottom_left = { pos.x, pos.y + size.y };
					ImVec2 bottom_right = { pos.x + size.x, pos.y + size.y };
					
					float left = 0.0f;
					float right = tex_size.x;
					float bottom = tex_size.y;
					float top = 0.f;
					float near_z = -1.0f;
					float far_z = 1.0f;
					
					//shape drawing
					std::string tooltip;
					for (const auto& displayed_tag : ctx_.current_project->displayed_tags)
					{
						auto& segment_storage = ctx_.get_current_segment_storage();
						auto it = segment_storage.find(displayed_tag);
						if (it == segment_storage.end()) continue;
						auto& tag = ctx_.current_project->tags.at(it->first);
						auto fill_color = (tag.color & ~0xFF000000) | 0x80000000;

						auto& segments = it->second;
						for (auto segment_it = segments.begin(); segment_it != segments.end(); ++segment_it)
						{
							auto& segment = *segment_it;
							bool is_onscreen = current_ts >= segment.start and current_ts <= segment.end;
							if (is_onscreen)
							{
								auto segment_attr_it = segment.attributes.find(video_data.id);
								if (segment_attr_it != segment.attributes.end())
								{
									for (auto& [attr_name, attr] : segment_attr_it->second)
									{
										if (!attr.has<shape>()) continue;

										bool is_selected = selected_attribute == &attr;
										bool show_points = is_selected;

										const auto& shape = attr.get<vt::shape>();
										draw_list->PushClipRect(top_left, bottom_right, true);
										shape.draw(current_ts, shape.interpolate, from_tex_pos, from_pixels, tex_size, size, is_selected ? orange : tag.color, fill_color, show_points, [&](size_t i)
										{
											if (ImGui::IsMouseClicked(0))
											{
												ctx_.video_timeline.selected_segment = widgets::selected_segment_data{ &tag, &segments, segment_it };
												ctx_.registry.execute<set_selected_attribute_command>(&attr);
											}
											tooltip = fmt::format("Tag: {}\nAttribute: {}\nID: {}", tag.name, attr_name, i + 1);
										});
									}
								}
							}
						}
					}

					if (hovered and !tooltip.empty() and !ImGuizmo::IsOver())
					{
						ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						widgets::tooltip(tooltip.c_str());
					}

					if (!is_keyframe and has_target)
					{
						ctx_.gizmo_target = nullptr;
					}

					if (last_focused and has_target and is_keyframe)
					{
						auto wpos = ImGui::GetWindowPos();
						auto wsize = ImGui::GetWindowSize();

						float translation[3]{ point_pos.x, point_pos.y, 0.0f };
						float rotation[3]{};
						float scale[3] = { 1.f, 1.f, 1.f };

						float target[3]
						{
							utils::matrix::front[0], //translation[0] + utils::matrix::front[0],
							utils::matrix::front[1], //translation[1] + utils::matrix::front[1],
							utils::matrix::front[2], //translation[2] + utils::matrix::front[2]
						};

						float cam_distance = 0.f;
						float cam_angle[2]{};
						float eye[3]
						{
							std::cos(cam_angle[1]) * std::cos(cam_angle[0]) * cam_distance,
							std::sin(cam_angle[0]) * cam_distance,
							std::sin(cam_angle[1]) * std::cosf(cam_angle[0]) * cam_distance
						};
						utils::matrix view_mat = (utils::matrix::look_at(eye, target));

						utils::matrix proj_mat = utils::matrix::ortho(left, right, bottom, top, near_z, far_z);
						ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

						utils::matrix mod{};
						auto& gizmo_style = ImGuizmo::GetStyle();
						gizmo_style.Colors[ImGuizmo::COLOR::DIRECTION_X] = ImGui::ColorConvertU32ToFloat4(0xFF1EC880);
						gizmo_style.Colors[ImGuizmo::COLOR::DIRECTION_Y] = ImGui::ColorConvertU32ToFloat4(0xFF503CF0);
						gizmo_style.Colors[ImGuizmo::COLOR::PLANE_Z] = ImGui::ColorConvertU32ToFloat4(0x80F08830);
						gizmo_style.Colors[ImGuizmo::COLOR::SELECTION] = ImGui::ColorConvertU32ToFloat4(orange);


						gizmo_style.CenterCircleSize = ctx_.app_settings.enable_gizmo_scaling ? from_pixels(5) : 5.f;
						gizmo_style.ScaleLineCircleSize = gizmo_style.CenterCircleSize;
						gizmo_style.TranslationLineThickness = 2.f * gizmo_style.CenterCircleSize / 3.f;
						gizmo_style.TranslationLineArrowSize = 1.5f * gizmo_style.TranslationLineThickness;
						ImGuizmo::SetOrthographic(true);
						ImGuizmo::SetDrawlist();

						float snap[3]{ 1.00f, 1.00f, 1.00f };
						ImVec2 obj_size{ 5, 100.f };
						float bounds[] = { -obj_size.y / 2, -obj_size.x / 2, 0.f, obj_size.y / 2, obj_size.x / 2, 0.f };
						ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, mod.data);
						if (ImGuizmo::Manipulate(view_mat.data, proj_mat.data, ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y/* | ImGuizmo::OPERATION::BOUNDS*/, ImGuizmo::MODE::LOCAL, mod.data, nullptr, snap, nullptr/*bounds*/))
						{
							ImGuizmo::DecomposeMatrixToComponents(mod.data, translation, rotation, scale);
							point_pos.x = std::clamp(translation[0], 0.0f, tex_size.x);
							point_pos.y = std::clamp(translation[1], 0.0f, tex_size.y);

							ctx_.gizmo_target->at(0) = (uint32_t)point_pos.x;
							ctx_.gizmo_target->at(1) = (uint32_t)point_pos.y;
						}
					}

					//window focus frame
					if (is_shape and last_focused and ctx_.last_focused_video.has_value())
					{
						draw_list->AddRect(top_left, bottom_right, orange, 0, 0, border_thickness);
					}
					//auto local_pos = from_tex_pos(point_pos);
					//draw_list->AddCircle(local_pos, 10.f, border_color);
				});
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
			ctx_.video_timeline.set_start_timestamp(timestamp::zero());
			ctx_.video_timeline.set_end_timestamp(timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(group_duration)));
			ctx_.video_timeline.set_current_timestamp(timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.current_timestamp()) });
			ctx_.video_timeline.insert_segment_container = &ctx_.insert_segment_data;

			ctx_.video_timeline.render(ctx_.win_cfg.show_timeline_window);

			if (ctx_.video_timeline.current_timestamp().total_milliseconds != std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.current_timestamp()))
			{
				ctx_.displayed_videos.seek(ctx_.video_timeline.current_timestamp().total_milliseconds);
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
				static auto rename_failed_popup = [](const std::string& id, const widgets::tag_rename_data& data, tag_validate_result fail_reason)
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
						ImGui::Text("Failed to rename tag \"%s\" to \"%s\"", data.old_name.c_str(), data.new_name.c_str());

						std::string error_text;
						switch (fail_reason)
						{
						case vt::tag_validate_result::already_exists: error_text = fmt::format("Tag \"{}\" already exists", data.new_name); break;
						case vt::tag_validate_result::invalid_name: error_text = "Invalid name"; break;
						case vt::tag_validate_result::too_long: error_text = fmt::format("Name can be at most {} characters long", tag_storage::max_tag_name_length); break;
						default: error_text = "Invalid name"; break;
						}
						ImGui::TextDisabled(error_text.c_str());
						ImGui::NewLine();
						auto area_size = ImGui::GetWindowSize();

						ImGui::SetCursorPosX(area_size.x / 2 - button_size.x / 2);
						if (ImGui::Button("OK", button_size))
						{
							return_value = true;
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					ImGui::PopStyleVar(2);

					return return_value;
				};

				static tag_validate_result fail_reason{};

				if (!tag_rename->processed)
				{
					tag_rename->processed = true;

					auto rename_result = ctx_.current_project->rename_tag(tag_rename->old_name, tag_rename->new_name);

					if (!rename_result.inserted)
					{
						//TODO: Display popup
						ImGui::OpenPopup("Rename Failed");
						fail_reason = rename_result.validation_result;
					}
					else
					{
						tag_rename.reset();
					}
				}
				
				if (tag_rename.has_value() and rename_failed_popup("Rename Failed", *tag_rename, fail_reason))
				{
					tag_rename.reset();
				}

			}
			if (tag_delete.has_value() and tag_delete->ready)
			{
				ctx_.current_project->delete_tag(tag_delete->tag);

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

		if (ctx_.win_cfg.show_shape_attributes_window)
		{
			ctx_.shape_attributes.render(ctx_.video_timeline.selected_segment, ctx_.win_cfg.show_shape_attributes_window);
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

		if (ctx_.win_cfg.show_console_window)
		{
			bool clear_console = ctx_.app_settings.clear_console_on_run;
			ctx_.console.render(ctx_.win_cfg.show_console_window, ctx_.app_settings.clear_console_on_run, ctx_.script_dir_filepath);
			if (clear_console != ctx_.app_settings.clear_console_on_run)
			{
				ctx_.settings["clear-console-on-run"] = ctx_.app_settings.clear_console_on_run;
				save_settings();
			}
		}

		if (ctx_.win_cfg.show_script_progress)
		{
			ctx_.script_progress.open();
			ctx_.script_progress.render(ctx_.win_cfg.show_script_progress);
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
		}
	}

	void main_window::enable_undocking(bool value)
	{
		ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
		auto node = ImGui::DockBuilderGetNode(dockspace_id);
		if (node != nullptr)
		{
			if (value)
			{
				node->LocalFlags |= ImGuiDockNodeFlags_NoUndocking;
			}
			else
			{
				node->LocalFlags &= ~ImGuiDockNodeFlags_NoUndocking;
			}
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

		auto dock_flags = ctx_.app_settings.enable_undocking ? dockspace_flags : (dockspace_flags | ImGuiDockNodeFlags_NoUndocking);
		if (ctx_.reset_layout and ImGui::DockBuilderGetNode(dockspace_id) != nullptr)
		{
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, dock_flags);
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			auto dockspace_id_copy = dockspace_id;
			auto main_dock_right = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id_copy);
			auto main_dock_up = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Up, 0.7f, nullptr, &dockspace_id_copy);
			auto main_dock_up_left = ImGui::DockBuilderSplitNode(main_dock_up, ImGuiDir_Left, 0.25f, nullptr, &main_dock_up);
			auto main_dock_down = ImGui::DockBuilderSplitNode(main_dock_up, ImGuiDir_Down, 0.25f, nullptr, &main_dock_up);
			auto dock_right_up = ImGui::DockBuilderSplitNode(main_dock_right, ImGuiDir_Up, 0.5f, nullptr, &main_dock_right);
			
			ImGui::DockBuilderDockWindow(widgets::inspector_id.c_str(), dock_right_up);
			ImGui::DockBuilderDockWindow(widgets::tag_manager_window_name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(widgets::shape_attributes::window_name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(widgets::video_group_queue::window_name().c_str(), main_dock_down);
			ImGui::DockBuilderDockWindow("Video Player", main_dock_up);
			ImGui::DockBuilderDockWindow(widgets::video_browser::window_name().c_str(), main_dock_up_left);
			ImGui::DockBuilderDockWindow("Theme Customizer", main_dock_up);
			ImGui::DockBuilderDockWindow(widgets::console::window_name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow(widgets::video_timeline::window_name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow("Video Group Browser", dockspace_id_copy);

			auto queue_node = ImGui::DockBuilderGetNode(main_dock_down);
			queue_node->LocalFlags = ImGuiDockNodeFlags_NoResizeY;

			ImGui::DockBuilderFinish(dockspace_id);
			ctx_.reset_layout = false;
		}

		ImGui::DockSpace(dockspace_id, ImVec2{}, dock_flags);
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
