#include <pch.hpp>
#include "main_window.hpp"
#include "app_context.hpp"
#include <fmt/format.h>

#include <widgets/widgets.hpp>
#include <widgets/tag_manager.hpp>
#include <widgets/video_widget.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/console.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/theme_customizer.hpp>
#include <ui/windows/inspector.hpp>
#include <ui/popups/options_popup.hpp>
#include <widgets/shape_attributes.hpp>
#include <widgets/localization_editor.hpp>
#include <widgets/video_group_queue.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/controls.hpp>
#include <widgets/modal/keybind_popup.hpp>
#include <widgets/modal/keybind_options_popup.hpp>
#include <widgets/insert_segment_popup.hpp>
#include <widgets/timeline.hpp>
#include <ui/icons.hpp>
#include <embeds/about.hpp>

#include <utils/filesystem.hpp>
#include <ImGuizmo.h>
#include <utils/matrix.hpp>
#include <utils/vec.hpp>
#include <utils/intersection.hpp>
#include <utils/string.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/slider.hpp>
#include <ui/widgets/settings_expander.hpp>

#include <events/system/window/system_window_resize_event.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segments_move_event.hpp>
#include <events/timeline/segment_merge_event.hpp>
#include <events/timeline/segment_delete_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_insert_event.hpp>
#include <events/tags/tag_add_request_event.hpp>
#include <events/tags/tag_add_event.hpp>
#include <events/tags/tag_rename_request_event.hpp>
#include <events/tags/tag_rename_event.hpp>
#include <events/tags/tag_delete_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>
#include <events/timeline/end_segment_drag_event.hpp>

#ifndef VT_VERSION
	#error VT_VERSION is not defined
#endif

extern "C"
{
	#include <libavutil/ffversion.h>
}

#include <openssl/opensslv.h>
#include <pybind11/pybind11.h>
#include <events/player/playback_changed_event.hpp>
#include <events/player/looping_changed_event.hpp>
#include <events/player/speed_changed_event.hpp>
#include <events/player/skip_next_event.hpp>
#include <events/player/skip_previous_event.hpp>
#include <events/player/seek_event.hpp>
#include <events/project_selector/open_project_event.hpp>
#include <events/project_selector/project_list_changed_event.hpp>
#include <events/system/window/system_window_drop_path_event.hpp>
#include <events/system/window/system_window_close_event.hpp>
#include <events/app/request_save_settings_event.hpp>

#ifdef _DEBUG
	#include <ui/windows/sandbox.hpp>
#endif
#include <events/filesystem/fetch_themes_event.hpp>
#include <events/filesystem/fetch_scripts_event.hpp>

namespace vt
{
	//TODO: tag_manager needs to be reworked and this removed
	static bool show_tag_rename_failed_popup = false;
	static std::optional<widgets::tag_rename_data> tag_rename;
	static std::optional<widgets::tag_delete_data> tag_delete;
	static tag_validate_result tag_rename_failed_reason;

	static void show_debug_info()
	{
		SDL_version compiled;
		SDL_version linked;
		SDL_VERSION(&compiled);
		SDL_GetVersion(&linked);
		debug::log("VideoTagger Version: {}", VT_VERSION);
		debug::log("SDL Version (Header):  {}.{}.{}", compiled.major, compiled.minor, compiled.patch);
		debug::log("SDL Version (Linked):  {}.{}.{}", linked.major, linked.minor, linked.patch);
		debug::log("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
		debug::log("ImGui Version: {}", IMGUI_VERSION);
		debug::log("FFmpeg Version: {}", FFMPEG_VERSION);
		debug::log("OpenSSL Version: {}", OPENSSL_FULL_VERSION_STR);
		debug::log("Python Version: {}", PY_VERSION);
		debug::log("pybind11 Version: {}.{}.{}", PYBIND11_VERSION_MAJOR, PYBIND11_VERSION_MINOR, PYBIND11_VERSION_PATCH);
	}

	main_window::main_window(const system_window_config& cfg) : system_window{ cfg }, event_source_{ "main_window" }
	{
		register_listeners();

		show_debug_info();

		init_options();
		load_settings();
		if (ctx_.first_launch)
		{
			on_first_launch();
		}
		ctx_.load_lang_packs("en_US");

		init_keybinds();
		init_player();
		fetch_themes();
		load_accounts();
		on_launch();

		ctx_.project_selector.load_projects_file(ctx_.projects_list_filepath);
	}

	void main_window::register_listeners()
	{
		ctx_.add_event_listener<system_window_close_event>([this](const system_window_close_event& event)
		{
			if (!event.is_from(*this)) return;
			on_close_project(true);
		});

		ctx_.add_event_listener<request_save_settings_event>([this](const request_save_settings_event& event)
		{
			save_settings();
		});

		ctx_.add_event_listener<system_window_drop_path_event>([this](const system_window_drop_path_event& event)
		{
			auto& path = event.path();
			debug::log("File dropped: {}, position: {}", path.u8string(), event.drop_point());
		});

		ctx_.add_event_listener<open_project_event>([this](const open_project_event& event)
		{
			auto project_info = event.project();

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
						ctx_.dispatch_event<project_list_changed_event>(event_source_);
					}
					break;
					case 2:
					{
						utils::dialog_filter filter{ "VideoTagger Project", project::extension };
						auto result = utils::filesystem::get_file({}, { filter });
						if (result)
						{
							project_info = project_info::load_from_file(result.path);
							ctx_.dispatch_event<project_list_changed_event>(event_source_);
						}
					}
					break;
				}
				return;
			}
			ctx_.current_project = project::load_from_file(project_info.path);
			ctx_.main_window->set_subtitle(ctx_.current_project->name);
			ctx_.get_window<widgets::console>().clear();
		});

		ctx_.add_event_listener<project_list_changed_event>([this](const project_list_changed_event& event)
		{
			ctx_.project_selector.sort();
			ctx_.project_selector.save_projects_file(ctx_.projects_list_filepath);
			debug::log("Saving projects list to {}", std::filesystem::absolute(ctx_.projects_list_filepath).u8string());
		});

		ctx_.add_event_listener<system_window_resize_event>([](const system_window_resize_event& event)
		{
			debug::log("Main window resized to {}x{}", event.width(), event.height());
		});

		ctx_.add_event_listener<segments_move_request_event>([event_source = event_source_](const segments_move_request_event& event)
		{
			auto& storage = event.storage();

			if (!event.ignore_conflicts())
			{
				segment_id_map conflicting_segments;
				for (const auto& [tag, segment_ids] : event.segments())
				{
					const auto& tag_timeline = storage.at(tag);
					for (auto& id : segment_ids)
					{
						const auto& segment = tag_timeline.at(id);
						auto segment_start = segment.start + (event.move_part() & segment_part::left ? event.move_offset() : timestamp::zero());
						auto segment_end = segment.end + (event.move_part() & segment_part::right ? event.move_offset() : timestamp::zero());

						auto range = tag_timeline.find_range(segment_start, segment_end);
						if (!range.empty())
						{
							if (range.size() == 1 && range.begin()->id == id)
							{
								continue;
							}

							conflicting_segments[tag].insert(id);
						}
					}
				}

				if (!conflicting_segments.empty())
				{
					ctx_.segments_move_conflict_popup = ui::new_popup<ui::segments_move_conflict_popup>(event, conflicting_segments);
					return;
				}
			}

			for (const auto& [tag, segment_ids] : event.segments())
			{
				auto move_results = storage.at(tag).move_offset(segment_ids, event.move_part(), event.move_offset());

				for (auto& move_result : move_results)
				{
					for (auto& merged_id : move_result.merged_segments())
					{
						ctx_.dispatch_event<segment_merge_event>(event_source, storage, tag, merged_id, move_result.resulting_segment());
					}
				}
			}

			ctx_.dispatch_event<segments_move_event>(event_source, event.storage(), event.segments(), event.move_part(), event.move_offset(), true);
			ctx_.is_project_dirty = true;
		});

		ctx_.add_event_listener<segment_delete_event>([](const segment_delete_event& event)
		{
			auto& storage = event.storage();
			auto it = storage.find(event.tag());
			if (it == storage.end()) return;

			it->second.erase(event.id());
			ctx_.is_project_dirty = true;
		});

		ctx_.add_event_listener<segment_insert_request_event>([event_source = event_source_](const segment_insert_request_event& event)
		{
			if (event.user_customization() or !event.tag().has_value())
			{
				auto max_ts = timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.duration()));

				ctx_.segment_insert_popup = ui::new_popup<ui::segment_insert_popup>(event, ctx_.current_project->displayed_tags, timestamp::zero(), max_ts);
				return;
			}

			auto& storage = ctx_.get_current_segment_storage();
			const auto& tag_name = *event.tag();
			auto it = storage.find(tag_name);
			auto& tag_timeline = it->second;
			if (it == storage.end()) return;

			if (!event.ignore_conflicts())
			{
				std::set<segment_id> conflicting_segments;
				for (auto& [id, _] : tag_timeline.find_range(event.start(), event.end()))
				{
					conflicting_segments.insert(id);
				}

				if (!conflicting_segments.empty())
				{
					ctx_.segment_insert_conflict_popup = ui::new_popup<ui::segment_insert_conflict_popup>(event, conflicting_segments);
					return;
				}
			}

			auto insert_result = tag_timeline.insert(event.start(), event.end());
			if (!insert_result.inserted())
			{
				ctx_.dispatch_event<segment_insert_event>(event_source, storage, tag_name, event.start(), event.end(), insert_result.preventing_segment(), false);
				return;
			}

			ctx_.dispatch_event<segment_insert_event>(event_source, storage, tag_name, event.start(), event.end(), insert_result.inserted_segment(), true);

			for (auto& merged_id : insert_result.merged_segments())
			{
				ctx_.dispatch_event<segment_merge_event>(event_source, storage, tag_name, merged_id, insert_result.inserted_segment());
			}

			ctx_.is_project_dirty = true;
		});

		ctx_.add_event_listener<tag_add_request_event>([event_source = event_source_](const tag_add_request_event& event)
		{
			auto& storage = event.storage();
			auto [it, validate_result] = storage.insert(event.tag_name(), event.color());
			if (validate_result == tag_validate_result::ok)
			{
				ctx_.is_project_dirty = true;
			}

			ctx_.dispatch_event<tag_add_event>(event_source, storage, event.tag_name(), validate_result);
		});

		ctx_.add_event_listener<tag_add_event>([](const tag_add_event& event)
		{
			if (event.validate_result() != tag_validate_result::ok) return;

			ctx_.current_project->add_displayed_tag(event.tag_name());
		});

		ctx_.add_event_listener<tag_rename_request_event>([event_source = event_source_](const tag_rename_request_event& event)
		{
			if (!tag_rename.has_value()) return;

			tag_rename->processed = true;

			auto rename_result = ctx_.current_project->rename_tag(tag_rename->old_name, tag_rename->new_name);

			if (!rename_result.inserted)
			{
				show_tag_rename_failed_popup = true;
				tag_rename_failed_reason = rename_result.validation_result;
			}
			else
			{
				tag_rename.reset();
			}

			ctx_.dispatch_event<tag_rename_event>(event_source, event.storage(), event.tag_name(), event.new_name(), rename_result);
		});

		ctx_.add_event_listener<tag_delete_event>([](const tag_delete_event& event)
		{
			if (!tag_delete.has_value()) return;

			ctx_.current_project->delete_tag(tag_delete->tag);

			tag_delete.reset();
		});

		ctx_.add_event_listener<segment_insert_mark_start>([](const segment_insert_mark_start& event)
		{
			ctx_.insert_segment_marks.push_back({ event.tag(), event.timestamp(), event.mark_id() });
		});

		ctx_.add_event_listener<segment_insert_mark_end>([event_source = event_source_](const segment_insert_mark_end& event)
		{
			auto it = ctx_.find_insert_segment_mark_by_id(event.mark_id());
			if (it == ctx_.insert_segment_marks.end()) return;

			ctx_.dispatch_event<segment_insert_request_event>(event_source, event.storage(), it->tag, it->start, event.timestamp(), event.user_customization(), false);

			ctx_.insert_segment_marks.erase(it);
		});

		ctx_.add_event_listener<end_segment_drag_event>([event_source = event_source_](const end_segment_drag_event& event)
		{
			auto [segments_min_ts, segments_max_ts] = event.min_max_timestamp();
			auto min_ts = timestamp::zero();
			auto max_ts = ctx_.displayed_videos.duration_as_timestamp();

			auto final_offset = event.final_offset();

			if (event.grab_part() & segment_part::left)
			{
				auto current_min_pos = segments_min_ts + final_offset;

				if (current_min_pos < min_ts)
				{
					final_offset -= current_min_pos - min_ts;
				}
				else if (current_min_pos > max_ts)
				{
					final_offset -= current_min_pos - max_ts;
				}
			}
			if (event.grab_part() & segment_part::right)
			{
				auto current_max_pos = segments_max_ts + final_offset;
				if (current_max_pos < min_ts)
				{
					final_offset -= current_max_pos - min_ts;
				}
				else if (current_max_pos > max_ts)
				{
					final_offset -= current_max_pos - max_ts;
				}
			}

			ctx_.dispatch_event<segments_move_request_event>(event_source, event.storage(), event.segments(), event.grab_part(), final_offset, false);
		});

		ctx_.add_event_listener<fetch_themes_event>([this](const fetch_themes_event& event)
		{
			auto priority = task_priority::low;
			ctx_.tasks.run([this]()
			{
				debug::log("Fetching themes...");
				auto result = fetch_themes();
				debug::log("Finished fetching themes");
				return result;
			}, priority)
			.then(ctx_.tasks.main_thread(), [](const utils::file_node& theme_list)
			{
				debug::log("Updating theme list");
				ctx_.themes = theme_list;
			}, priority);
		});

		ctx_.add_event_listener<fetch_scripts_event>([this](const fetch_scripts_event& event)
		{
			ctx_.tasks.run([this]()
			{
				debug::log("Fetching scripts...");
				auto result = fetch_scripts(ctx_.script_dir_filepath);
				debug::log("Finished fetching scripts");
				return result;
			}, task_priority::low)
			.then(ctx_.tasks.main_thread(), [](const utils::file_node& script_list)
			{
				debug::log("Updating script list");
				ctx_.scripts = script_list;
			}, task_priority::low);
		});
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
		ctx_.set_selected_attribute(nullptr);

		//Save window size & state
		{
			auto& json_window = ctx_.settings["window"];
			if (ctx_.win_cfg.state == window_state::normal)
			{
				auto& size_setting = json_window["size"];
				auto win_size = size();
				size_setting["width"] = win_size[0];
				size_setting["height"] = win_size[1];
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
		auto& segments = ctx_.get_current_segment_storage().at(ctx_.video_timeline.selected_segment->tag);
		if (segments.is_id_valid(ctx_.video_timeline.selected_segment->segment_id))
		{
			segments.erase(ctx_.video_timeline.selected_segment->segment_id);
			ctx_.video_timeline.selected_segment.reset();
		}
	}

	void main_window::on_launch()
	{
		ctx_.dispatch_event<fetch_themes_event>(event_source_);
		ctx_.dispatch_event<fetch_scripts_event>(event_source_);
	}

	void main_window::on_first_launch()
	{
		ctx_.reset_layout = true;
		copy_app_assets();
		ctx_.settings["first-launch"] = false;
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
					set_size({ size["width"].get<int>(), size["height"].get<int>() });
					center();
				}
			}
			if (ctx_.settings.contains("window") and ctx_.settings["window"].contains("state"))
			{
				auto& json_window = ctx_.settings["window"];
				auto state = json_window["state"].get<window_state>();
				switch (state)
				{
					case window_state::maximized: maximize(); break;
					default: break;
				}
				ctx_.win_cfg.state = state;
			}
			if (ctx_.settings.contains("first-launch"))
			{
				ctx_.first_launch = ctx_.settings["first-launch"];				
			}
			if (ctx_.settings.contains("show-windows"))
			{
				auto& show_windows = ctx_.settings["show-windows"];

				if (show_windows.contains("tag-manager")) ctx_.win_cfg.show_tag_manager_window = show_windows["tag-manager"];
				if (show_windows.contains("timeline")) ctx_.win_cfg.show_timeline_window = show_windows["timeline"];
			}

			if (ctx_.settings.contains("preferences"))
			{
				auto& json_preferences = ctx_.settings["preferences"];
				ctx_.deserialize_app_settings(json_preferences);
			}

			if (ctx_.settings.contains("windows"))
			{
				auto& json_windows = ctx_.settings["windows"];
				ctx_.deserialize_windows(json_windows);
			}
		}
		else
		{
			ctx_.reset_layout = true;
		}

		ctx_.change_theme(theme::load_from_file(ctx_.theme_dir_filepath / fmt::format("dark.{}", theme::extension)));

		auto& io = ImGui::GetIO();
		if (!std::filesystem::exists(io.IniFilename))
		{
			ctx_.reset_layout = true;
		}
		build_fonts(font_size);
		return result;
	}

	void main_window::save_settings()
	{
		if (!ctx_.app_settings_filepath.empty())
		{
			debug::log("Saving app settings...");

			auto json_preferences = ctx_.serialize_app_settings();
			if (!json_preferences.empty())
			{
				ctx_.settings["preferences"] = json_preferences;
			}

			auto json_windows = ctx_.serialize_windows();
			if (!json_windows.empty())
			{
				ctx_.settings["windows"] = json_windows;
			}

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

	void main_window::copy_app_assets()
	{
		std::filesystem::path assets_path = "assets";
		auto lang_path = assets_path / "lang";
		debug::log("Copying builtin assets...");

		if (!std::filesystem::exists(ctx_.lang_dir_filepath))
		{
			std::filesystem::create_directories(ctx_.lang_dir_filepath);
		}

		for (const auto& entry : std::filesystem::directory_iterator(lang_path))
		{
			if (entry.is_directory()) continue;
			auto source = entry.path();
			auto target = ctx_.lang_dir_filepath / source.filename();
			debug::log("Copied asset {} -> {}", source.u8string(), target.u8string());
			std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
		}
	}

	void main_window::init_keybinds()
	{
		ctx_.keybinds.clear();

		keybind_flags flags(true, false, false);
		ctx_.keybinds.insert(ctx_.lang->get("project.save").c_str(), keybind(SDLK_s, keybind_modifiers{true}, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save();
		})));

		ctx_.keybinds.insert(ctx_.lang->get("project.save_as").c_str(), keybind(SDLK_s, keybind_modifiers{true, true}, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save_as();
		})));

		ctx_.keybinds.insert(ctx_.lang->get("show_in_explorer").c_str(), keybind(SDLK_o, keybind_modifiers{true, false, true}, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_show_in_explorer();
		})));

		ctx_.keybinds.insert(ctx_.lang->get("import_videos").c_str(), keybind(SDLK_i, keybind_modifiers{true}, flags,
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

		ctx_.keybinds.insert(ctx_.lang->get("project.close").c_str(), keybind(SDLK_F4, keybind_modifiers{true}, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			close_project();
		})));

		ctx_.keybinds.insert(ctx_.lang->get("exit").c_str(), keybind(SDLK_F4, keybind_modifiers{false, false, true}, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_close_project(true);
		})));

		keybind_modifiers toggle_window_mod{ false, true };
		//TODO: Reimplement these
		//ctx_.keybinds.insert("Toggle Video Player", keybind(SDLK_F1, toggle_window_mod, flags, toggle_window_action("video-player", ctx_.win_cfg.show_video_player_window)));
		//ctx_.keybinds.insert("Toggle Video Browser", keybind(SDLK_F2, toggle_window_mod, flags, toggle_window_action("video-browser", ctx_.win_cfg.show_video_browser_window)));
		//ctx_.keybinds.insert("Toggle Video Group Browser", keybind(SDLK_F3, toggle_window_mod, flags, toggle_window_action("video-group-browser", ctx_.win_cfg.show_video_group_browser_window)));
		//ctx_.keybinds.insert("Toggle Video Group Queue", keybind(SDLK_F4, toggle_window_mod, flags, toggle_window_action("video-group-queue", ctx_.win_cfg.show_video_group_queue_window)));
		//ctx_.keybinds.insert("Toggle Inspector", keybind(SDLK_F5, toggle_window_mod, flags, toggle_window_action("inspector", ctx_.win_cfg.show_inspector_window)));
		//ctx_.keybinds.insert("Toggle Shape Attributes", keybind(SDLK_F6, toggle_window_mod, flags, toggle_window_action("shape-attributes", ctx_.win_cfg.show_shape_attributes_window)));
		ctx_.keybinds.insert("Toggle Tag Manager", keybind(SDLK_F7, toggle_window_mod, flags, toggle_window_action("tag-manager", ctx_.win_cfg.show_tag_manager_window)));
		ctx_.keybinds.insert("Toggle Timeline", keybind(SDLK_F8, toggle_window_mod, flags, toggle_window_action("timeline", ctx_.win_cfg.show_timeline_window)));
		//ctx_.keybinds.insert("Toggle Console", keybind(SDLK_F9, toggle_window_mod, flags, toggle_window_action("console", ctx_.win_cfg.show_console_window)));

		keybind_modifiers player_mod{};
		ctx_.keybinds.insert("Play/Pause", keybind(SDLK_SPACE, player_mod, flags, player_action(player_action_type::play_pause)));

		keybind_modifiers player_secondary_mod{ false, true };
		ctx_.keybinds.insert("Seek Forwards", keybind(SDLK_RIGHT, player_mod, flags, player_action(player_action_type::forwards)));
		ctx_.keybinds.insert("Seek Backwards", keybind(SDLK_LEFT, player_mod, flags, player_action(player_action_type::backwards)));
		ctx_.keybinds.insert("Skip To Next", keybind(SDLK_RIGHT, player_secondary_mod, flags, player_action(player_action_type::skip_next)));
		ctx_.keybinds.insert("Skip To Previous", keybind(SDLK_LEFT, player_secondary_mod, flags, player_action(player_action_type::skip_previous)));
		ctx_.keybinds.insert("Toggle Looping", keybind(SDLK_l, keybind_modifiers{ true }, flags, player_action(player_action_type::toggle_looping)));


		//TODO: Maybe move this somewhere else
		ctx_.add_event_listener<tag_delete_event>([](const tag_delete_event& event)
		{
			auto& keybinds = ctx_.current_project->keybinds;
			for (auto it = keybinds.begin(); it != keybinds.end();)
			{
				auto& kb = it->second;
				if (!kb.flags.removable)
				{
					++it;
					continue;
				}

				auto timeline_action_ptr = std::dynamic_pointer_cast<timeline_action>(kb.action);
				if (timeline_action_ptr == nullptr)
				{
					++it;
					continue;
				}

				if (timeline_action_ptr->tag() != event.tag_name())
				{
					++it;
					continue;
				}

				it = keybinds.erase(it);
			}
		});
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
				if (ui::button("Add Keybind", { avail.x, ImGui::GetTextLineHeightWithSpacing() * 1.5f * io.FontGlobalScale }))
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
						if (ui::checkbox(("##KeybindEnabled" + name).c_str(), enabled))
						{
							keybind.flags.enabled = enabled;
							ctx_.is_project_dirty = true;
						}

						ImGui::SameLine();
						ImGui::PushID(id);
						if (!is_row_selected) ImGui::BeginDisabled();
						std::string delete_kb_id = fmt::format("{}##DeleteKb{}", icons::delete_, name);
						if (ui::icon_button(delete_kb_id.c_str()))
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
							if (ui::icon_button(edit_id.c_str()))
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
							if (ui::icon_button(edit_combination_id.c_str()))
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
								if (ui::icon_button(edit_kb_id.c_str()))
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

		options("Application Settings", "General").add_raw([this]()
		{
			ui::label("Font Size");
			ImGui::SameLine();
			int start_font_size = static_cast<int>(ctx_.get_font()->FontSize);
			static int font_size = start_font_size;
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("000").x);
			//TODO: Add messagebox informing that the changes will be applied only after restart
			if (ImGui::DragInt("##FontSize", &font_size, 1.0f, 8, 72, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				ctx_.app_settings.font_size = font_size;
			}

			float text_height = ImGui::GetTextLineHeight();

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Thumbnail Size");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 4);
			if (ImGui::DragFloat("##ThumbnailSizeDrag", &ctx_.app_settings.thumbnail_size, 0.5f, 45.0f, 100.f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
			{
				
			}
			return true;
		})
		.add_toggle("Load Thumbnails", "Specifies whether to load thumbnails when opening a project", ctx_.app_settings.load_thumbnails, [&](bool value)
		{
			//ctx_.settings["load-thumbnails"] = ctx_.app_settings.load_thumbnails;
		})
		.add_button("Clear Thumbnails Cache", "Clears the thumbnails cache", "Clear", []()
		{
			std::filesystem::remove_all(ctx_.cache_dir_filepath);
			std::filesystem::create_directories(ctx_.cache_dir_filepath);
		})
		.add_label_spacer("UI")
		.add_toggle("Scale Gizmos", "Scales gizmos size based on viewport size", ctx_.app_settings.scale_gizmos, [&](bool value)
		{
			//ctx_.settings["scale-gizmos"] = ctx_.app_settings.scale_gizmos;
		})

		//TODO: Add theme selection

#ifdef _DEBUG
		.add_label_spacer("Debug Only")
		.add_raw([]()
		{
			auto& io = ImGui::GetIO();
			static constexpr auto accent_color = ImVec4{ 0.2588f, 0.6f, 0.8784f, 1.f };
			static constexpr auto accent_color_hover = ImVec4{ 0.2f, 0.5098f, 0.7804f, 1.f };

			auto avail_area = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent_color_hover);
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accent_color);
			static ui::slider<float> font_slider(0.5f, 2.0f, io.FontGlobalScale, { avail_area.x, ImGui::GetFrameHeight() });
			font_slider.set_step(0.005f);
			font_slider.render_with_label("Font Scale");
			font_slider.set_on_change_callback([&io](float old_value, float value)
			{
				io.FontGlobalScale = value;
			});
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
			return true;
		})
#endif
		;

		options("Application Settings", "Keybinds").add_raw([]()
		{
			display_keybinds_panel(ctx_.keybinds, false, false, false);
			return true;
		});

		options("Project Settings", "Keybinds").add_raw([]()
		{
			display_keybinds_panel(ctx_.current_project->keybinds);
			return true;
		});

		options("Storage Settings", "Accounts").add_raw([]()
		{
			//TODO: maybe make this a widget
			
			//TODO: move this somewhere so it can have a value per account
			static bool login_in_progress = false;
			bool modifed_account = false;

			static std::map<std::string, std::future<bool>> retry_login_futures;

			for (auto& [service_id, account_manager] : ctx_.account_managers)
			{
				ImGui::PushFont(ctx_.get_font(font_type::h3));
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
							if (ui::icon_button(icons::retry))
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
			return true;
		});

		options.set_active_tab("Application Settings", "General");
	}

	void main_window::init_player()
	{
		auto& player = ctx_.get_window<widgets::video_player>();

		player.callbacks.on_set_playing = [&player, event_source = event_source_](bool is_playing)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_playing(is_playing);
			//}
			if (ctx_.current_video_group_id() == invalid_video_group_id) return;
			ctx_.dispatch_event<playback_changed_event>(event_source, player, is_playing);

			//TODO: This should be handled as a playback_changed_event listener
			ctx_.displayed_videos.set_playing(is_playing);
		};

		player.callbacks.on_set_looping = [&player, event_source = event_source_](loop_mode mode)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_looping(is_looping);
			//}
			if (ctx_.current_video_group_id() == invalid_video_group_id) return;
			ctx_.dispatch_event<looping_changed_event>(event_source, player, mode);
		};

		player.callbacks.on_set_speed = [&player, event_source = event_source_](float speed)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_speed(speed);
			//}

			if (ctx_.current_video_group_id() == invalid_video_group_id) return;
			ctx_.dispatch_event<speed_changed_event>(event_source, player, speed);

			//TODO: This should be handled as a speed_changed_event listener
			ctx_.displayed_videos.set_speed(speed);
		};

		player.callbacks.on_skip = [&player, event_source = event_source_](int dir, loop_mode mode, bool is_playing)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;

			video_group_playlist::iterator it;
			if (dir > 0)
			{
				it = playlist.next();
				ctx_.dispatch_event<skip_next_event>(event_source, player);
			}
			else if (dir < 0)
			{
				it = playlist.previous();
				ctx_.dispatch_event<skip_previous_event>(event_source, player);
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

		player.callbacks.on_seek = [&player, event_source = event_source_](std::chrono::nanoseconds ts)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.seek(ts);
			//}

			if (ctx_.current_video_group_id() == invalid_video_group_id) return;
			ctx_.dispatch_event<seek_event>(event_source, player, ts);
			ctx_.displayed_videos.seek(ts);
		};

		player.callbacks.on_finish = [&player](loop_mode mode, bool is_playing)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;
			//TODO: Consider adding an event here

			if (mode == loop_mode::one)
			{
				ctx_.displayed_videos.seek(std::chrono::nanoseconds{ 0 });
				ctx_.displayed_videos.set_playing(true);
				return;
			}

			auto& player = ctx_.get_window<widgets::video_player>();
			if (player.should_autoplay())
			{
				player.reset_data();

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

	utils::file_node main_window::fetch_themes()
	{
		utils::file_node result;
		auto& dark = result["dark"];
		auto& light = result["light"];

		if (!std::filesystem::is_directory(ctx_.theme_dir_filepath)) return result;
		for (auto& dir_entry : std::filesystem::directory_iterator(ctx_.theme_dir_filepath))
		{
			if (!dir_entry.is_regular_file()) continue;

			auto path = dir_entry.path();
			if (path.extension() != fmt::format(".{}", theme::extension)) continue;

			auto temp_theme = theme::load_from_file(path);
			(temp_theme.is_dark() ? dark : light).insert(path);
		}
		return result;
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
			if (ui::begin_main_menu(ctx_.lang->get("menu_bar.file").c_str()))
			{
				std::string key_name = ctx_.lang->get("import_videos").c_str();
				auto& kb = ctx_.keybinds.at(key_name);
				std::string menu_name = fmt::format("{} {}", icons::import_, key_name);
				if (ui::begin_menu(menu_name.c_str()))
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

					ui::end_menu();
				}
				ImGui::Separator();
				{
					std::string key_name = ctx_.lang->get("project.save").c_str();
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = fmt::format("{} {}", icons::save, key_name);
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_save();
					}
				}

				{

					std::string key_name = ctx_.lang->get("project.save_as").c_str();
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
						std::string key_name = ctx_.lang->get("show_in_explorer").c_str();
						auto& kb = ctx_.keybinds.at(key_name);
						std::string menu_name = fmt::format("{} {}", icons::folder, ctx_.lang->get("show_in_explorer").c_str());
						if (ImGui::MenuItem(menu_name.c_str(), kb.name().c_str()))
						{
							on_show_in_explorer();
						}
					}

					{
						std::string menu_name = fmt::format("{} {}", icons::import_export, ctx_.lang->get("import_export").c_str());
						if (ui::begin_menu(menu_name.c_str()))
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
							ui::end_menu();
						}
					}
				}
				ImGui::Separator();
				{
					std::string key_name = "Close Project";
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = std::string(icons::close) + ' ' + ctx_.lang->get("project.close").c_str();
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						close_project();
					}
				}
				ImGui::Separator();
				{
					std::string key_name = "Exit";
					auto& kb = ctx_.keybinds.at(key_name);
					std::string menu_item = std::string(icons::exit) + ' ' + ctx_.lang->get("exit").c_str();
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_close_project(true);
					}
				}
				ui::end_menu();
			}

			//TODO: Enable this when undo/redo is implemented
			/*if (ImGui::BeginMenu(ctx_.lang->get_cstr("menu_bar.edit")))
			{
				if (ImGui::MenuItem(ctx_.lang->get_cstr("menu_bar.edit.undo"), nullptr, nullptr, false))
				{

				}

				if (ImGui::MenuItem(ctx_.lang->get_cstr("menu_bar.edit.redo"), nullptr, nullptr, false))
				{

				}
				ImGui::EndMenu();
			}*/

			if (ui::begin_main_menu(ctx_.lang->get("menu_bar.window").c_str()))
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
					//win_toggles{ "Show Video Player", "Toggle Video Player", "video-player", &ctx_.win_cfg.show_video_player_window },
					//win_toggles{ "Show Video Browser", "Toggle Video Browser", "video-browser", &ctx_.win_cfg.show_video_browser_window },
					//win_toggles{ "Show Video Group Browser", "Toggle Video Group Browser", "video-group-browser", &ctx_.win_cfg.show_video_group_browser_window },
					//win_toggles{ "Show Video Group Queue", "Toggle Video Group Queue", "video-group-queue", &ctx_.win_cfg.show_video_group_queue_window },
					win_toggles{},
					//win_toggles{ "Show Inspector", "Toggle Inspector", "inspector", &ctx_.win_cfg.show_inspector_window },
					//win_toggles{ "Show Shape Attributes", "Toggle Shape Attributes", "shape-attributes", &ctx_.win_cfg.show_shape_attributes_window },
					win_toggles{ "Show Tag Manager", "Toggle Tag Manager", "tag-manager", &ctx_.win_cfg.show_tag_manager_window },
					win_toggles{ "Show Timeline", "Toggle Timeline", "timeline", &ctx_.win_cfg.show_timeline_window },
					//win_toggles{ "Show Console", "Toggle Console", "console", &ctx_.win_cfg.show_console_window},

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
				
				for (auto& window : ctx_.registered_windows())
				{
					bool show_window = window->is_open();
					auto& icon = window->icon();
					std::string menu_name;
					if (window->has_icon())
					{
						menu_name = fmt::format("{} Show {}", icon, window->display_name());
					}
					else
					{
						menu_name = fmt::format("Show {}", window->display_name());
					}

					if (ImGui::MenuItem(menu_name.c_str(), nullptr, &show_window))
					{
						window->set_opened(show_window);
					}
				}
#endif

				ImGui::Separator();
				if (ImGui::MenuItem("Allow Undocking", nullptr, ctx_.app_settings.allow_undocking))
				{
					ctx_.app_settings.allow_undocking = !ctx_.app_settings.allow_undocking;
					enable_undocking(ctx_.app_settings.allow_undocking);
				}
				if (ImGui::MenuItem(ctx_.lang->get("redock_videos").c_str()))
				{
					ctx_.reset_player_docking = true;
				}
				if (ImGui::MenuItem(ctx_.lang->get("reset_layout").c_str()))
				{
					ctx_.reset_layout = true;
				}
				ui::end_menu();
			}

			if (ui::begin_main_menu(ctx_.lang->get("menu_bar.run").c_str()))
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

				if (ui::begin_menu(menu_name.c_str(), !ctx_.scripts.empty()))
				{
					std::function<void(const utils::file_node&)> draw_folder = [&draw_folder, &has_scripts](const utils::file_node& node)
					{
						for (const auto& [path, folder] : node)
						{
							auto dir_name = fmt::format("{} {}", icons::folder_code, path.stem().string());
							if (!folder.empty() and ui::begin_menu(dir_name.c_str()))
							{
								draw_folder(folder);
								ui::end_menu();
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
								ctx_.run_script(child);
							}
						}
					};

					draw_folder(ctx_.scripts);
					ui::end_menu();
				}

				menu_name = fmt::format("{} {}", icons::folder, "Open Scripts Folder");
				if (ImGui::MenuItem(menu_name.c_str(), nullptr, nullptr, std::filesystem::exists(ctx_.script_dir_filepath)))
				{
					utils::filesystem::open_in_explorer(std::filesystem::absolute(ctx_.script_dir_filepath));
				}
				ui::end_menu();
			}

			if (ui::begin_main_menu(ctx_.lang->get("menu_bar.tools").c_str()))
			{
				if (ui::begin_menu("Themes"))
				{
					const auto& theme_node = ctx_.themes;
					const auto& dark_themes = theme_node["dark"];
					const auto& light_themes = theme_node["light"];

					bool has_theme_dir = std::filesystem::exists(ctx_.theme_dir_filepath);
					if (ImGui::MenuItem("Reload"))
					{
						ctx_.dispatch_event<fetch_themes_event>(event_source_);
					}

					auto menu_name = fmt::format("{} {}", icons::folder, "Open Themes Folder");
					if (ImGui::MenuItem(menu_name.c_str(), nullptr, nullptr, has_theme_dir))
					{
						utils::filesystem::open_in_explorer(std::filesystem::absolute(ctx_.theme_dir_filepath));
					}
					ImGui::Separator();

					if (ui::begin_menu("Dark", !dark_themes.empty()))
					{
						for (const auto& path : dark_themes.children)
						{
							auto name = utils::string::to_titlecase(path.stem().u8string());
							if (ImGui::MenuItem(name.c_str()))
							{
								auto new_theme = theme::load_from_file(path);
								ctx_.change_theme(new_theme);
							}
						}
						ui::end_menu();
					}
					if (ui::begin_menu("Light", !light_themes.empty()))
					{
						for (const auto& path : light_themes.children)
						{
							auto name = utils::string::to_titlecase(path.stem().u8string());

							if (ImGui::MenuItem(name.c_str()))
							{
								auto new_theme = theme::load_from_file(path);
								ctx_.change_theme(new_theme);
							}
						}
						ui::end_menu();
					}
					ui::end_menu();
				}

				auto& theme_customizer = ctx_.get_window<widgets::theme_customizer>();
				bool show_theme_customizer = theme_customizer.is_open();
				if (ImGui::MenuItem("Theme Customizer", nullptr, &show_theme_customizer))
				{
					theme_customizer.set_opened(show_theme_customizer);
				}
				ImGui::Separator();
				if (ImGui::MenuItem(fmt::format("{} {}", icons::settings, ctx_.lang->get("menu_bar.tool.options")).c_str()))
				{
					ctx_.win_cfg.show_options_window = true;
				}
				ui::end_menu();
			}
			if (ui::begin_main_menu(ctx_.lang->get("menu_bar.help").c_str()))
			{
				if (ImGui::MenuItem(ctx_.lang->get("menu_bar.help.about").c_str()))
				{
					ctx_.win_cfg.show_about_window = true;
				}
				ui::end_menu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ctx_.win_cfg.show_about_window)
		{
			ImGui::OpenPopup("AboutPopup");
		}
		if (ctx_.win_cfg.show_options_window)
		{
			ctx_.options.open();
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
				const auto& theme = ctx_.current_theme;

				ImGui::PushFont(ctx_.get_font(font_type::h3));
				ImGui::TextUnformatted("About VideoTagger");
				ImGui::Separator();
				ImGui::Dummy(style.ItemSpacing);
				ImGui::PopFont();

				ImVec2 child_size = ImGui::GetContentRegionAvail();
				child_size.y -= ImGui::GetTextLineHeightWithSpacing() * 1.25f + style.WindowPadding.y;
				if (ImGui::BeginChild("##AboutScrollableArea", child_size))
				{
					ImGui::BeginDisabled();

					ImGui::Text("Version: %s", VT_VERSION);
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
					ImGui::PushFont(ctx_.get_font(font_type::h3));
					ImGui::TextUnformatted("Third Party Libraries");
					ImGui::PopFont();

					for (const auto& [name, license] : embed::third_party_licenses)
					{
						if (widgets::begin_collapsible(fmt::format("##{}", name), name, 0, icons::license))
						{
							ui::card([&]()
							{
								ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
								ImGui::TextWrapped("%s", license.c_str());
								ImGui::PopStyleColor();
							});
							widgets::end_collapsible();
						}
					}
				}
				ImGui::EndChild();

				auto button_size = ImVec2{ ImGui::GetContentRegionAvail().x, 0 };
				if (ui::accent_button("Close", button_size) or ImGui::IsKeyReleased(ImGuiKey_Escape))
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

		//TODO: Use new merge segments popup (segments_move_conflict_popup)
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
					ctx_.current_project->schedule_video_import(task.importer_id, std::move(import_data), utils::random::get_uuid());
				}
				it = tasks.erase(it);
			}
		}

		{
			auto& tasks = ctx_.current_project->video_import_tasks;
			for (auto it = tasks.begin(); it != tasks.end();)
			{
				auto& task = *it;
				if (task.task.wait_for(std::chrono::seconds{}) != std::future_status::ready)
				{
					++it;
					continue;
				}

				auto vid_resource = task.task.get();
				if (vid_resource != nullptr)
				{
					video_id_t video_id = vid_resource->id();
					if (ctx_.current_project->import_video(std::move(vid_resource), task.group_id))
					{
						if (ctx_.app_settings.load_thumbnails)
						{
							ctx_.current_project->schedule_load_thumbnail(video_id);
						}
					}
				}
				it = tasks.erase(it);
			}
		}

		{
			auto& tasks = ctx_.current_project->load_thumbnail_tasks;
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
			auto& console = ctx_.get_window<widgets::console>();
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
					console.add_entry(widgets::console::entry::flag_type::error, fmt::format("Failed to download video {} ({})", video_name, task.video_id), widgets::console::entry::source_info{ "VideoTagger", -1 });
				}
				else
				{
					debug::log("Downloaded video {} ({})", video_name, task.video_id);
					dynamic_cast<downloadable_video_resource&>(ctx_.current_project->videos.get(task.video_id)).set_file_path(task.task.data->download_path.u8string());
					console.add_entry(widgets::console::entry::flag_type::info, fmt::format("Downloaded video {} ({})", video_name, task.video_id), widgets::console::entry::source_info{ "VideoTagger", -1 });
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

		auto& player = ctx_.get_window<widgets::video_player>();
		if (player.is_open())
		{
			video_player_data data = player.data();
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
			//		player.update_data(data);
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
					player.update_data(data, false);

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
				player.update_data(data, ctx_.displayed_videos.is_playing());

				reset_player_when_group_is_invalid = true;
			}
		}

		/*uint64_t vid_id{};
		if (player.is_visible())
		{
			for (auto& [id, vinfo] : ctx_.current_project->videos)
			{
				if (!vinfo.is_widget_open) continue;
				auto& vid = vinfo.video;

				widgets::draw_video_widget(vid, vinfo.is_widget_open, vid_id++);
			}
		}*/

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
			
			widgets::draw_tag_manager_widget(ctx_.current_project->tags, tag_rename, tag_delete, ctx_.is_project_dirty, ctx_.win_cfg.show_tag_manager_window);

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
					ImGui::TextDisabled("%s", error_text.c_str());
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

			//TODO: rewrite to use the popup class
			if (show_tag_rename_failed_popup)
			{
				ImGui::OpenPopup("Rename Failed");
				if (rename_failed_popup("Rename Failed", *tag_rename, tag_rename_failed_reason))
				{
					tag_rename.reset();
				}
			}
		}

		//TODO: Add base virtual class that has render(bool&) method instead of this

		ctx_.tag_importer.render(ctx_.win_cfg.show_tag_importer_window);

		if (ctx_.win_cfg.show_options_window)
		{
			ctx_.options.render();
		}

		if (ctx_.win_cfg.show_script_progress)
		{
			ctx_.script_progress.open();
			ctx_.script_progress.render(ctx_.win_cfg.show_script_progress);
		}

		if (ctx_.segments_move_conflict_popup != nullptr)
		{
			if (!ctx_.segments_move_conflict_popup->is_open())
			{
				ctx_.segments_move_conflict_popup->open();
			}

			ctx_.segments_move_conflict_popup->render();
			if (!ctx_.segments_move_conflict_popup->is_open())
			{
				ctx_.segments_move_conflict_popup.reset();
			}
		}

		if (ctx_.segment_insert_conflict_popup != nullptr)
		{
			if (!ctx_.segment_insert_conflict_popup->is_open())
			{
				ctx_.segment_insert_conflict_popup->open();
			}

			ctx_.segment_insert_conflict_popup->render();
			if (!ctx_.segment_insert_conflict_popup->is_open())
			{
				ctx_.segment_insert_conflict_popup.reset();
			}
		}

		if (ctx_.segment_insert_popup != nullptr)
		{
			ctx_.segment_insert_popup->open();
			ctx_.segment_insert_popup->render();
			if (!ctx_.segment_insert_popup->is_open())
			{
				ctx_.segment_insert_popup.reset();
			}
		}

		auto& timeline = ctx_.get_window<widgets::timeline>();
		bool v = true;
		if (ctx_.current_video_group_id() != invalid_video_group_id)
		{
			auto& state = timeline.state();
			state.set_min_timestamp(timestamp::zero());
			state.set_max_timestamp(ctx_.displayed_videos.duration_as_timestamp());
			state.set_current_timestamp(ctx_.displayed_videos.current_timestamp_as_timestamp());
		}

		timeline.set_on_seek_callback([](timestamp ts)
		{
			if (ts != ctx_.displayed_videos.current_timestamp_as_timestamp())
			{
				ctx_.displayed_videos.seek(ts.total_milliseconds);
			}
		});
		
		ctx_.render_windows();
		draw_video_widgets();
		
		//ImGui::ShowDemoWindow();
		//ImGui::OpenPopup("Script Progress");

		//TODO: Maybe add a status bar here?
		//if (ImGui::BeginViewportSideBar("##VideoTaggerSidebar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetTextLineHeightWithSpacing() * 2, ImGuiWindowFlags_AlwaysAutoResize))
		//{
		//	ImGui::End();
		//}
	}

	void main_window::draw_video_widgets()
	{
		auto& player = ctx_.get_window<widgets::video_player>();

		//TODO: This breaks when there are undocked videos
		if (player.is_visible())
		{
			tag_attribute_instance* selected_attribute = ctx_.get_selected_attribute();
			bool has_selected_attribute = selected_attribute != nullptr;

			bool is_shape = has_selected_attribute and selected_attribute->has<shape>() and selected_attribute->get<shape>().get_type() != shape::type::none;
			if (ctx_.last_focused_video.has_value() and ctx_.displayed_videos.find(ctx_.last_focused_video.value()) == ctx_.displayed_videos.end())
			{
				ctx_.last_focused_video = std::nullopt;
				ctx_.set_selected_attribute(nullptr);
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

				widgets::draw_video_widget(video_data.video, video_data.display_texture, timestamp_in_range, is_widget_open, vid_id++, [&point_pos, has_selected_attribute, selected_attribute, is_shape, has_target, &video_data, &selected_segment](ImVec2 pos, ImVec2 size, ImVec2 tex_size)
				{
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

						if (ImGui::MenuItem(fmt::format("{} Add Keyframe", icons::add_keyframe).c_str(), nullptr, nullptr, !is_keyframe))
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
								if (ui::icon_button(icons::align_horizontal_center))
								{
									point_pos.x = tex_size.x / 2;
									close = true;
								}
								ui::tooltip("Align Horizontal Center");
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_vertical_top))
								{
									point_pos.y = 0;
									close = true;
								}
								ui::tooltip("Align Vertical Top");

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_horizontal_left))
								{
									point_pos.x = 0;
									close = true;
								}
								ui::tooltip("Align Horizontal Left");
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_center))
								{
									point_pos = tex_size / 2;
									close = true;
								}
								ui::tooltip("Align Center");
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_horizontal_right))
								{
									point_pos.x = tex_size.x;
									close = true;
								}
								ui::tooltip("Align Horizontal Right");

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_vertical_center))
								{
									point_pos.y = tex_size.y / 2;
									close = true;
								}
								ui::tooltip("Align Vertical Center");
								ImGui::TableNextColumn();
								if (ui::icon_button(icons::align_vertical_bottom))
								{
									point_pos.y = tex_size.y;
									close = true;
								}
								ui::tooltip("Align Vertical Bottom");

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
						for (auto& [segment_id, segment] : segments)
						{
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
										shape.draw(current_ts, shape.interpolate, from_tex_pos, from_pixels, tex_size, size, is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : tag.color, fill_color, show_points, [&](size_t i)
										{
											if (ImGui::IsMouseClicked(0) and hovered)
											{
												ctx_.video_timeline.selected_segment = widgets::selected_segment_data{ tag.name, segment_id };
												ctx_.set_selected_attribute(&attr);
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
						ui::tooltip(tooltip);
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
							std::sin(cam_angle[1]) * std::cos(cam_angle[0]) * cam_distance
						};
						utils::matrix view_mat = (utils::matrix::look_at(eye, target));

						utils::matrix proj_mat = utils::matrix::ortho(left, right, bottom, top, near_z, far_z);
						ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

						utils::matrix mod{};
						auto& gizmo_style = ImGuizmo::GetStyle();

						gizmo_style.CenterCircleSize = ctx_.app_settings.scale_gizmos ? from_pixels(5) : 5.f;
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
					if (selected_segment.has_value() and last_focused and ctx_.last_focused_video.has_value())
					{
						draw_list->AddRect(top_left, bottom_right, ctx_.current_theme.get_rgba(theme_color::selection_normal), 0, 0, border_thickness);
					}
					//auto local_pos = from_tex_pos(point_pos);
					//draw_list->AddCircle(local_pos, 10.f, border_color);
				});
			}
		}

		if (ctx_.reset_player_docking)
		{
			auto it = ctx_.current_project->video_groups.find(ctx_.current_video_group_id());
			if (it != ctx_.current_project->video_groups.end() and player.is_visible())
			{
				player.dock_windows(it->second.size());
				ctx_.reset_player_docking = false;
			}
		}
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

	void main_window::on_render()
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

		auto dock_flags = ctx_.app_settings.allow_undocking ? dockspace_flags : (dockspace_flags | ImGuiDockNodeFlags_NoUndocking);
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
			
			ImGui::DockBuilderDockWindow(ctx_.get_window<ui::windows::inspector>().name().c_str(), dock_right_up);
			ImGui::DockBuilderDockWindow(widgets::tag_manager_window_name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::shape_attributes>().name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_group_queue>().name().c_str(), main_dock_down);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_player>().name().c_str(), main_dock_up);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::localization_editor>().name().c_str(), main_dock_up);
#ifdef _DEBUG
			ImGui::DockBuilderDockWindow(ctx_.get_window<ui::windows::sandbox>().name().c_str(), main_dock_up);
#endif
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_browser>().name().c_str(), main_dock_up_left);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::theme_customizer>().name().c_str(), main_dock_up);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::console>().name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow(widgets::video_timeline::window_name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_group_browser>().name().c_str(), dockspace_id_copy);

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
		system_window::handle_event(event);

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
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						ctx_.dispatch_event<system_window_resize_event>(event_source_, *this, utils::vec2<uint32_t>{ (uint32_t)event.window.data1, (uint32_t)event.window.data2 });
					}
					break;
					case SDL_WINDOWEVENT_CLOSE:
					{
						ctx_.dispatch_event<system_window_close_event>(event_source_, *this);
					}
					break;
				}
			}
			break;
			case SDL_DROPFILE:
			{
				if (event.drop.windowID != SDL_GetWindowID(window)) return;
				if (event.drop.file != nullptr)
				{
					auto drop_point = ImGui::GetMousePos();
					auto win_pos = position();

					auto win_drop_pos = utils::vec2<float>{ drop_point.x - win_pos[0], drop_point.y - win_pos[1] };					
					ctx_.dispatch_event<system_window_drop_path_event>(event_source_, *this, event.drop.file, win_drop_pos);
				}
			}
			break;
		}
	}
}
