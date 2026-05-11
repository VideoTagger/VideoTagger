#include <pch.hpp>
#include "main_window.hpp"
#include "app_context.hpp"
#include <fmt/format.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <ui/windows/tag_manager.hpp>
#include <widgets/video_player.hpp>
#include <widgets/console.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/theme_customizer.hpp>
#include <ui/windows/inspector.hpp>
#include <ui/windows/video_window.hpp>
#include <ui/popups/options_popup.hpp>
#include <ui/windows/region_properties.hpp>
#include <widgets/localization_editor.hpp>
#include <widgets/video_group_queue.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/controls.hpp>
#include <widgets/modal/keybind_popup.hpp>
#include <widgets/modal/keybind_options_popup.hpp>
#include <widgets/timeline.hpp>
#include <ui/icons.hpp>
#include <embeds/about.hpp>
#include <embeds/dark_theme.hpp>

#include <utils/filesystem.hpp>
#include <ImGuizmo.h>
#include <utils/matrix.hpp>
#include <utils/vec.hpp>
#include <utils/intersection.hpp>
#include <utils/string.hpp>
#include <system/messagebox.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/themed_slider.hpp>
#include <ui/widgets/settings_expander.hpp>

#include <updates/update_manager.hpp>

#include <events/system/window/system_window_resize_event.hpp>

#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segments_moved_event.hpp>
#include <events/timeline/segment_merged_event.hpp>
#include <events/timeline/segment_delete_request_event.hpp>
#include <events/timeline/segment_deleted_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_inserted_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/timeline/end_segment_drag_event.hpp>
#include <events/timeline/segment_select_request_event.hpp>
#include <events/timeline/segment_selected_event.hpp>
#include <events/timeline/segment_deselect_request_event.hpp>
#include <events/timeline/segment_deselected_event.hpp>
#include <events/timeline/segment_select_all_request_event.hpp>
#include <events/timeline/segment_deselect_all_request_event.hpp>
#include <events/interceptors/update_segment_drag_interceptor.hpp>

#include <events/tags/tag_add_request_event.hpp>
#include <events/tags/tag_added_event.hpp>
#include <events/tags/tag_rename_request_event.hpp>
#include <events/tags/tag_renamed_event.hpp>
#include <events/tags/tag_delete_request_event.hpp>
#include <events/tags/tag_deleted_event.hpp>
#include <events/tags/tag_change_display_request_event.hpp>
#include <events/tags/tag_display_changed_event.hpp>

#include <events/gizmo/gizmo_move_targets_event.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>

#include <events/scripts/script_end_event.hpp>

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
#include <events/player/playback_change_request_event.hpp>
#include <events/player/looping_changed_event.hpp>
#include <events/player/looping_change_request_event.hpp>
#include <events/player/speed_changed_event.hpp>
#include <events/player/speed_change_request_event.hpp>
#include <events/player/skip_next_request_event.hpp>
#include <events/player/skip_previous_request_event.hpp>
#include <events/player/seek_event.hpp>
#include <events/player/seek_request_event.hpp>
#include <events/player/seek_to_end_request_event.hpp>
#include <events/player/seek_to_start_request_event.hpp>
#include <events/player/seek_to_previous_frame_request_event.hpp>
#include <events/player/seek_to_next_frame_request_event.hpp>
#include <events/player/video_group_change_request_event.hpp>
#include <events/player/video_group_changed_event.hpp>
#include <events/player/playback_reached_end_event.hpp>

#include <events/project_selector/open_project_event.hpp>
#include <events/project_selector/project_list_changed_event.hpp>
#include <events/system/window/system_window_drop_path_event.hpp>
#include <events/system/window/system_window_close_event.hpp>
#include <events/app/request_save_settings_event.hpp>
#include <core/platform.hpp>

#ifdef VT_DEBUG
	#include <ui/windows/sandbox.hpp>
#endif
#include <events/filesystem/fetch_themes_event.hpp>
#include <events/filesystem/fetch_scripts_event.hpp>

#include <video/google_drive/google_drive_video_importer.hpp>
#include <video/local_video_importer.hpp>

#include <events/video_resource/google_drive_video_import_request_event.hpp>
#include <events/video_resource/local_video_import_request_event.hpp>
#include <events/video_resource/video_imported_event.hpp>
#include <events/video_resource/video_load_thumbnail_request_event.hpp>
#include <events/video_resource/video_open_importer_request_event.hpp>
#include <events/video_resource/video_start_download_request_event.hpp>
#include <events/video_resource/video_cancel_download_request_event.hpp>
#include <events/video_resource/video_download_started_event.hpp>
#include <events/video_resource/video_refresh_request_event.hpp>
#include <events/video_resource/video_delete_request_event.hpp>
#include <events/video_resource/video_deleted_event.hpp>
#include <events/video_resource/video_delete_downloaded_file_request_event.hpp>
#include <events/video_resource/video_downloaded_file_deleted_event.hpp>
#include <events/video_resource/video_download_canceled_event.hpp>
#include <events/video_resource/video_download_finished_event.hpp>
#include <events/video_resource/video_open_in_explorer_request_event.hpp>
#include <events/video_resource/video_locate_request_event.hpp>
#include <events/attributes/region_select_request_event.hpp>
#include <events/attributes/region_deselect_request_event.hpp>
#include <events/attributes/attribute_add_request_event.hpp>
#include <events/attributes/attribute_added_event.hpp>
#include <events/attributes/attribute_delete_request_event.hpp>
#include <events/attributes/attribute_deleted_event.hpp>
#include <events/attributes/attribute_rename_request_event.hpp>
#include <events/attributes/attribute_renamed_event.hpp>


namespace vt
{
	template <typename importer_type, typename... import_args>
	void handle_video_import_request(import_args&&... args)
	{
		if (!ctx_.is_video_importer_registered<importer_type>())
		{
			return;
		}

		ctx_.session.tasks.run([args...]() mutable
		{
			auto& importer = ctx_.get_video_importer<importer_type>();
			return importer.import_video(std::forward<import_args>(args)...);
		})
		.then(ctx_.session.tasks.on_main(), [](const std::shared_ptr<video_resource>& vid_res)
		{
			if (vid_res == nullptr)
			{
				return;
			}

			video_id_t video_id = vid_res->id();
			if (!ctx_.current_project->import_video(vid_res, utils::random::get_uuid()))
			{
				return;
			}

			if (ctx_.app_settings.load_thumbnails)
			{
				ctx_.dispatch_event<video_load_thumbnail_request_event>("main_window", video_id, false, true);
			}
			ctx_.dispatch_event<video_imported_event>("main_window", video_id);
		});
	}

	static void show_debug_info()
	{
		SDL_version compiled{};
		SDL_version linked{};
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
		register_video_resource_listeners();
		register_attribute_listeners();

		show_debug_info();

		init_options();
		load_settings();
		if (ctx_.first_launch)
		{
			on_first_launch();
		}

		init_keybinds();
		init_player();
		load_accounts();
		on_launch();

		ctx_.project_selector.load_projects_file(ctx_.projects_list_filepath);
	}

	void main_window::register_listeners()
	{
		ctx_.add_event_listener<script_end_event>([this](const script_end_event& event)
		{
			auto tb = taskbar_proxy();
			tb.reset();
		});

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
				messagebox_data data{};
				data.icon = messagebox_icon::info;
				data.title = "VideoTagger";
				data.buttons =
				{
					{ 0, ctx_.lang->get("cancel") },
					{ 1, ctx_.lang->get("remove") },
					{ 2, ctx_.lang->get("locate") },
				};
				data.message = "This project no longer exists";
				data.cancel_button_id = 0;
				data.default_button_id = 1;
				data.callback = [this, project_info](int button_id)
				{
					switch (button_id)
					{
						case 1:
						{
							ctx_.project_selector.remove(project_info);
						}
						break;
						case 2:
						{
							utils::dialog_filter filter{ "VideoTagger Project", project::extension };
							auto result = utils::filesystem::get_file({}, { filter });
							if (result)
							{
								auto& pinfo = ctx_.project_selector.replace(project_info, project_info::load_from_file(result.path));
								load_project(pinfo);
							}
						}
						break;
					}
				};
				messagebox::show(data);
			}
			else
			{
				load_project(project_info);
			}
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
				// Doesn't detect a conflict if the moved segments overlap with each other
				segment_id_map conflicting_segments;
				for (const auto& [tag, segment_ids] : event.segments())
				{
					const auto& tag_timeline = storage.at(tag);
					auto conflicts = tag_timeline.find_move_conflicts(segment_ids, event.move_part(), event.move_offset());
					if (!conflicts.empty())
					{
						conflicting_segments.emplace(tag, std::move(conflicts));
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
						ctx_.dispatch_event<segment_merged_event>(event_source, storage, tag, merged_id, move_result.resulting_segment());
					}
				}
			}

			ctx_.dispatch_event<segments_moved_event>(event_source, event.storage(), event.segments(), event.move_part(), event.move_offset(), true);
			ctx_.is_project_dirty = true;
		});

		ctx_.add_event_listener<segment_delete_request_event>([event_source = event_source_](const segment_delete_request_event& event)
		{
			bool deleted = false;
			auto& storage = event.storage();
			auto it = storage.find(event.tag());
			if (it != storage.end())
			{
				deleted = it->second.erase(event.id());
				if (deleted)
				{
					ctx_.is_project_dirty = true;
				}
			}

			ctx_.dispatch_event<segment_deleted_event>(event_source, storage, event.tag(), event.id(), deleted);
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
				ctx_.dispatch_event<segment_inserted_event>(event_source, storage, tag_name, event.start(), event.end(), insert_result.preventing_segment(), false);
				return;
			}

			ctx_.dispatch_event<segment_inserted_event>(event_source, storage, tag_name, event.start(), event.end(), insert_result.inserted_segment(), true);

			for (auto& merged_id : insert_result.merged_segments())
			{
				ctx_.dispatch_event<segment_merged_event>(event_source, storage, tag_name, merged_id, insert_result.inserted_segment());
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

			ctx_.dispatch_event<tag_added_event>(event_source, storage, event.tag_name(), validate_result);
		});

		ctx_.add_event_listener<tag_added_event>([event_source = event_source_](const tag_added_event& event)
		{
			if (!event.added()) return;

			ctx_.dispatch_event<tag_change_display_request_event>(event_source, event.storage(), event.tag_name(), true);
		});

		ctx_.add_event_listener<tag_rename_request_event>([event_source = event_source_](const tag_rename_request_event& event)
		{
			auto& project = *ctx_.current_project;

			auto rename_result = project.tags.rename(event.tag_name(), event.new_name());

			if (!rename_result.inserted)
			{
				//TODO: maybe should display a popup
			}
			else
			{
				ctx_.is_project_dirty = true;
			}

			ctx_.dispatch_event<tag_renamed_event>(event_source, event.storage(), event.tag_name(), event.new_name(), rename_result);
		});

		ctx_.add_event_listener<tag_renamed_event>([event_source = event_source_](const tag_renamed_event& event)
		{
			if (!event.renamed())
			{
				ctx_.tag_rename_failed_popup = ui::new_popup<ui::tag_rename_failed_popup>(event);
				return;
			}

			auto& project = *ctx_.current_project;
			const auto& old_name = event.tag_name();
			const auto& new_name = event.new_name();

			if (auto it = project.find_displayed_tag(old_name); it != project.displayed_tags.end())
			{
				project.displayed_tags.erase(it);
				project.add_displayed_tag(new_name);
			}

			for (auto& [group_id, group] : project.video_groups)
			{
				auto& segments = group.segments();
				auto node_handle = segments.extract(old_name);
				if (!node_handle.empty())
				{
					node_handle.key() = new_name;
					segments.insert(std::move(node_handle));
				}
			}
		});

		ctx_.add_event_listener<tag_delete_request_event>([event_source = event_source_](const tag_delete_request_event& event)
		{
			auto& tags = ctx_.current_project->tags;
			bool deleted = tags.erase(event.tag_name());
			if (deleted)
			{
				ctx_.is_project_dirty = true;
			}

			ctx_.dispatch_event<tag_deleted_event>(event_source, event.storage(), event.tag_name(), deleted);
		});

		ctx_.add_event_listener<tag_deleted_event>([event_source = event_source_](const tag_deleted_event& event)
		{
			if (!event.deleted()) return;

			auto& project = *ctx_.current_project;

			auto current_group_id = ctx_.session.current_video_group_id();

			//TODO: change after segment events store the group id.
			// Should also send events but currently everything assumes that the segment in the event belongs to the current video group
			for (auto& [group_id, group] : project.video_groups)
			{
				if (group_id == current_group_id) continue;

				auto& group_segments = group.segments();
				auto segments_it = group_segments.find(event.tag_name());
				if (segments_it != group_segments.end())
				{
					group_segments.erase(segments_it);
				}
			}

			auto& current_segments = ctx_.get_current_segment_storage();
			auto segments_it = current_segments.find(event.tag_name());
			if (segments_it != current_segments.end())
			{
				auto& segments = segments_it->second;
				std::vector<segment_id> segments_to_delete;
				segments_to_delete.reserve(segments.size());
				for (auto& [id, _] : segments)
				{
					segments_to_delete.push_back(id);
				}

				for (auto& id : segments_to_delete)
				{
					ctx_.dispatch_event<segment_delete_request_event>(event_source, current_segments, event.tag_name(), id);
				}
			}

			ctx_.dispatch_event<tag_change_display_request_event>(event_source, event.storage(), event.tag_name(), false);
		});

		ctx_.add_event_listener<tag_change_display_request_event>([event_source = event_source_](const tag_change_display_request_event& event)
		{
			auto& project = *ctx_.current_project;
			if (event.display())
			{
				if (!project.add_displayed_tag(event.tag_name())) return;
			}
			else
			{
				if (!project.remove_displayed_tag(event.tag_name())) return;
			}

			ctx_.dispatch_event<tag_display_changed_event>(event_source, event.storage(), event.tag_name(), event.display());
		});

		ctx_.add_event_interceptor<update_segment_drag_event, update_segment_drag_interceptor>();

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
				auto result = fetch_themes(ctx_.theme_dir_filepath);
				debug::log("Finished fetching themes");
				return result;
			}, priority)
			.then(ctx_.tasks.on_main(), [](const utils::file_node& theme_list)
			{
				debug::log("Updating theme list");
				ctx_.themes = theme_list;
			}, std::nullopt, priority);
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
			.then(ctx_.tasks.on_main(), [](const utils::file_node& script_list)
			{
				debug::log("Updating script list");
				ctx_.scripts = script_list;
			}, std::nullopt, task_priority::low);
		});
	}

	void main_window::register_player_listeners()
	{
		//TODO: maybe all these listeners should have highest priority

		auto& player = ctx_.get_window<widgets::video_player>();

		ctx_.add_event_listener<playback_change_request_event>([&player, this](const playback_change_request_event& event)
		{
			if (&event.player() != &player) return;

			if (event.is_playing() == ctx_.displayed_videos.is_playing()) return;

			ctx_.displayed_videos.set_playing(event.is_playing());
			ctx_.dispatch_event<playback_changed_event>(event.source(), player, event.is_playing());
		});

		ctx_.add_event_listener<seek_request_event>([&player, this](const seek_request_event& event)
		{
			if (&event.player() != &player) return;

			ctx_.displayed_videos.seek(event.timestamp());
			ctx_.dispatch_event<seek_event>(event.source(), player, event.timestamp());
		});

		ctx_.add_event_listener<seek_to_start_request_event>([&player, this](const seek_to_start_request_event& event)
		{
			if (&event.player() != &player) return;

			ctx_.dispatch_event<seek_request_event>(event.source(), player, std::chrono::nanoseconds::zero());
		});

		ctx_.add_event_listener<seek_to_end_request_event>([&player, this](const seek_to_end_request_event& event)
		{
			if (&event.player() != &player) return;

			ctx_.dispatch_event<seek_request_event>(event.source(), player, ctx_.displayed_videos.duration());
		});

		ctx_.add_event_listener<seek_to_previous_frame_request_event>([&player, this](const seek_to_previous_frame_request_event& event)
		{
			if (&event.player() != &player) return;
			
			ctx_.dispatch_event<seek_request_event>(event.source(), player, ctx_.displayed_videos.previous_frame_timestamp());
		});

		ctx_.add_event_listener<seek_to_next_frame_request_event>([&player, this](const seek_to_next_frame_request_event& event)
		{
			if (&event.player() != &player) return;

			ctx_.dispatch_event<seek_request_event>(event.source(), player, ctx_.displayed_videos.next_frame_timestamp());
		});

		ctx_.add_event_listener<looping_change_request_event>([&player, this](const looping_change_request_event& event)
		{
			if (&event.player() != &player) return;

			//TODO: video queue should be in charge of looping and skipping,

			ctx_.dispatch_event<looping_changed_event>(event.source(), player, event.mode());
		});

		ctx_.add_event_listener<speed_change_request_event>([&player, this](const speed_change_request_event& event)
		{
			if (&event.player() != &player) return;

			ctx_.displayed_videos.set_speed(event.speed());

			ctx_.dispatch_event<speed_changed_event>(event.source(), player, ctx_.displayed_videos.speed());
		});

		ctx_.add_event_listener<skip_next_request_event>([&player, this](const skip_next_request_event& event)
		{
			//TODO: video queue should be in charge of looping and skipping,

			if (&event.player() != &player) return;

			auto& playlist = ctx_.current_project->video_group_playlist;
			if (playlist.empty()) return;

			auto it = playlist.next();

			if (player.loop_mode() == loop_mode::all and it == playlist.end())
			{
				ctx_.dispatch_event<video_group_change_request_event>(event.source(), player, *playlist.begin());
				return;
			}

			if (it != playlist.end())
			{
				ctx_.dispatch_event<video_group_change_request_event>(event.source(), player, *it);
				return;
			}

			ctx_.dispatch_event<video_group_change_request_event>(event.source(), player, invalid_video_group_id);
		});

		ctx_.add_event_listener<skip_previous_request_event>([&player, this](const skip_previous_request_event& event)
		{
			//TODO: video queue should be in charge of looping and skipping,

			if (&event.player() != &player) return;

			auto& playlist = ctx_.current_project->video_group_playlist;
			if (playlist.empty()) return;

			auto it = playlist.previous();

			if (it == playlist.begin() or it == playlist.end()) return

			ctx_.dispatch_event<video_group_change_request_event>(event.source(), player, *it);
		});

		ctx_.add_event_listener<video_group_changed_event>([&player, this](const video_group_changed_event& event)
		{
			ctx_.displayed_videos.clear();

			auto& playlist = ctx_.current_project->video_group_playlist;

			if (event.new_group_id() == invalid_video_group_id)
			{
				playlist.set_current(playlist.end());
				return;
			}

			auto playlist_it = playlist.find(event.new_group_id());

			if (playlist_it != playlist.end())
			{
				playlist.set_current(playlist_it);
			}

			for (auto& group_inf : ctx_.current_project->video_groups.at(event.new_group_id()))
			{
				auto vid_resource = ctx_.current_project->videos.get(group_inf.id);
				if (vid_resource == nullptr)
				{
					debug::error("Video resource with id {} not found for video group {}", group_inf.id, event.new_group_id());
					continue;
				}

				if (!vid_resource->playable())
				{
					debug::error("Video {} with hash {} is not available", vid_resource->title(), vid_resource->sha256());
					continue;
				}

				ctx_.displayed_videos.insert(group_inf.id, vid_resource->video(), group_inf.offset, vid_resource->width(), vid_resource->height());
			}

			ctx_.reset_player_docking = true;
			player.focus();
		}, event_listener_priority::highest);

		ctx_.add_event_listener<playback_reached_end_event>([&player, this](const playback_reached_end_event& event)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;
			auto event_source = player.get_event_source();

			if (player.loop_mode() == loop_mode::one)
			{
				ctx_.dispatch_event<seek_to_start_request_event>(event_source, player);
				ctx_.dispatch_event<playback_change_request_event>(event_source, player, true);
				return;
			}

			if (player.should_autoplay())
			{
				ctx_.dispatch_event<skip_next_request_event>(event_source, player);
			}
		});
	}

	void main_window::register_video_resource_listeners()
	{
		ctx_.add_event_listener<google_drive_video_import_request_event>([this](const google_drive_video_import_request_event& event)
		{
			handle_video_import_request<google_drive_video_importer>(video_importer::generate_video_id(), event.file_id());
		});

		ctx_.add_event_listener<local_video_import_request_event>([this](const local_video_import_request_event& event)
		{
			handle_video_import_request<local_video_importer>(video_importer::generate_video_id(), event.filepath());
		});

		ctx_.add_event_listener<video_load_thumbnail_request_event>([this](const video_load_thumbnail_request_event& event)
		{
			struct thumbnail_load_result
			{
				video_resource_thumbnail thumbnail;
				bool from_cache;
			};

			cancellation_token token;
			std::set<std::string> task_tags{ "video_resource", video_id_to_task_tag(event.video_id()) };

			ctx_.session.tasks.run([video_id = event.video_id(), ignore_cache = event.ignore_cache()](cancellation_token& token) mutable -> std::optional<thumbnail_load_result>
			{
				auto vid_res = ctx_.current_project->videos.get(video_id);
				if (vid_res == nullptr)
				{
					debug::error("Video resource with id {} not found", video_id);
					return std::nullopt;
				}

				std::filesystem::path thumbnail_path = ctx_.thumbnail_dir_filepath / vid_res->sha256();

				if (token.is_cancelled())
				{
					return std::nullopt;
				}

				if (!ignore_cache)
				{
					int image_width;
					int image_height;
					int image_channels;
					uint8_t* image_data = stbi_load(thumbnail_path.u8string().c_str(), &image_width, &image_height, &image_channels, 3);
					if (image_data != nullptr)
					{
						if (image_channels != 3)
						{
							debug::error("Thumbnail image {} has invalid number of channels: {}", thumbnail_path.u8string(), image_channels);
							stbi_image_free(image_data);
							return std::nullopt;
						}

						video_resource_thumbnail thumbnail
						{
							std::vector<uint8_t>(image_data, image_data + image_width * image_height * image_channels),
							image_width,
							image_height
						};
						stbi_image_free(image_data);

						return thumbnail_load_result{ std::move(thumbnail), true };
					}
				}

				if (token.is_cancelled())
				{
					return std::nullopt;
				}

				auto thumbnail = vid_res->generate_thumbnail();
				if (!thumbnail.has_value())
				{
					debug::error("Failed to generate thumbnail for video {}", video_id);
					return std::nullopt;
				}

				return thumbnail_load_result{ std::move(*thumbnail), false };
			}, token, task_tags)
			.then([video_id = event.video_id(), cache_result = event.cache_result()](const std::optional<thumbnail_load_result>& load_result, cancellation_token& token) -> std::optional<thumbnail_load_result>
			{
				if (!load_result.has_value())
				{
					return load_result;
				}

				auto vid_res = ctx_.current_project->videos.get(video_id);
				if (vid_res == nullptr)
				{
					debug::error("Video resource with id {} not found", video_id);
					return std::nullopt;
				}
				std::filesystem::path thumbnail_path = ctx_.thumbnail_dir_filepath / vid_res->sha256();
				const auto& [thumbnail, from_cache] = *load_result;

				if (token.is_cancelled())
				{
					return std::nullopt;
				}

				if (cache_result and !from_cache)
				{
					std::filesystem::create_directories(ctx_.thumbnail_dir_filepath);
					if (!stbi_write_png(thumbnail_path.u8string().c_str(), thumbnail.width, thumbnail.height, 3, thumbnail.pixels.data(), thumbnail.width * 3))
					{
						debug::error("Failed to save thumbnail to {}", thumbnail_path.u8string());
					}
				}

				return load_result;
			}, token, task_tags)
			.then(ctx_.tasks.on_main(), [video_id = event.video_id()](const std::optional<thumbnail_load_result>& load_result)
			{
				if (!load_result.has_value())
				{
					return;
				}

				auto& [thumbnail, from_cache] = *load_result;
				auto vid_res = ctx_.current_project->videos.get(video_id);
				if (vid_res == nullptr)
				{
					debug::error("Video resource with id {} not found", video_id);
					return;
				}
				vid_res->set_thumbnail(thumbnail.texture());
			}, std::nullopt, task_tags);
		});

		ctx_.add_event_listener<video_start_download_request_event>([this](const video_start_download_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get<downloadable_video_resource>(event.video_id());
			if (vid_res == nullptr)
			{
				debug::error("Video with id {} is not a downloadable resource", event.video_id());
				return;
			}

			if (vid_res->downloadable() != video_downloadable_status::downloadable)
			{
				debug::error("Video with id {} is not downloadable right now", event.video_id());
				return;
			}

			cancellation_token token;
			std::set<std::string> task_tags{ "video_resource", video_id_to_task_tag(event.video_id()), "download"};

			ctx_.session.tasks.run([vid_res](cancellation_token& token)
			{
				return vid_res->download(token);
			}, token, task_tags)
			.then(ctx_.tasks.on_main(), [vid_res](const video_download_result& download_result)
			{
				switch (download_result.status)
				{
					case video_download_status::completed:
						if (vid_res->file_path() != download_result.download_path)
						{
							vid_res->set_file_path(download_result.download_path.u8string());
							ctx_.is_project_dirty = true;
						}
						ctx_.dispatch_event<video_download_finished_event>("video_resource", vid_res->id(), true);
						break;

					case video_download_status::failed:
						ctx_.dispatch_event<video_download_finished_event>("video_resource", vid_res->id(), false);
						break;

					case video_download_status::cancelled:
						ctx_.dispatch_event<video_download_canceled_event>("video_resource", vid_res->id());
						break;

					default: break;
				}
			}, std::nullopt, task_tags);

			ctx_.dispatch_event<video_download_started_event>(event_source_, event.video_id());
		});

		ctx_.add_event_listener<video_download_finished_event>([this](const video_download_finished_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get<downloadable_video_resource>(event.video_id());
			if (vid_res == nullptr)
			{
				debug::error("Video with id {} is not a downloadable resource", event.video_id());
				return;
			}

			if (event.successful() and !vid_res->has_thumbnail())
			{
				ctx_.dispatch_event<video_load_thumbnail_request_event>("video_resource", event.video_id(), false, true);
			}
		});

		ctx_.add_event_listener<video_cancel_download_request_event>([this](const video_cancel_download_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get<downloadable_video_resource>(event.video_id());
			if (vid_res == nullptr)
			{
				debug::error("Video with id {} is not a downloadable resource", event.video_id());
				return;
			}

			ctx_.session.tasks.cancel_with_all({ "video_resource", video_id_to_task_tag(event.video_id()), "download" });
		});

		ctx_.add_event_listener<video_refresh_request_event>([this](const video_refresh_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get(event.video_id());
			if (vid_res == nullptr)
			{
				debug::error("Video resource with id {} not found", event.video_id());
				return;
			}

			std::set<std::string> task_tags{ "video_resource", video_id_to_task_tag(event.video_id()) };

			if (vid_res->can_async_refresh())
			{
				ctx_.session.tasks.run([vid_res]()
				{
					vid_res->refresh();
				}, task_tags);
			}
			else
			{
				ctx_.session.tasks.run_on_main([vid_res]()
				{
					vid_res->refresh();
				}, task_tags);
			}
		});

		ctx_.add_event_listener<video_delete_request_event>([this](const video_delete_request_event& event)
		{
			auto vid = ctx_.current_project->videos.get(event.video_id());
			if (vid == nullptr)
			{
				return;
			}
			vid->mark_for_removal();

			ctx_.session.tasks.run([video_id = event.video_id()]()
			{
				std::set<std::string> task_tags = { "video_resource", video_id_to_task_tag(video_id) };

				ctx_.session.tasks.cancel_with_all(task_tags);
				ctx_.session.tasks.await_with_all(task_tags);
			})
			.then(ctx_.tasks.on_main(), [this, video_id = event.video_id()]()
			{
				ctx_.current_project->remove_video(video_id);
				ctx_.dispatch_event<video_deleted_event>(event_source_, video_id, true);
			});
		});

		ctx_.add_event_listener<video_delete_downloaded_file_request_event>([this](const video_delete_downloaded_file_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get<downloadable_video_resource>(event.video_id());
			if (vid_res == nullptr)
			{
				debug::error("Video with id {} is not a downloadable resource", event.video_id());
				return;
			}

			if (!vid_res->remove_downloaded_file())
			{
				return;
			}

			ctx_.dispatch_event<video_downloaded_file_deleted_event>(event_source_, event.video_id());
		});

		ctx_.add_event_listener<video_open_in_explorer_request_event>([this](const video_open_in_explorer_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get(event.video_id());
			if (vid_res == nullptr) return;

			auto absolute_path = std::filesystem::absolute(vid_res->file_path());
			utils::filesystem::open_file_in_explorer(absolute_path);
		});

		ctx_.add_event_listener<video_locate_request_event>([this](const video_locate_request_event& event)
		{
			auto vid_res = ctx_.current_project->videos.get(event.video_id());
			if (vid_res == nullptr) return;

			static utils::dialog_filters filters
			{
				{ "Video", utils::filesystem::concat_extensions(std::vector<std::string>(ctx_.valid_video_extensions.begin(), ctx_.valid_video_extensions.end())) },
			};

			auto absolute_path = std::filesystem::absolute(vid_res->file_path());
			auto result = utils::filesystem::get_file(absolute_path, filters);
			if (!result) return;

			auto hash = utils::hash::sha256_file(result.path);
			if (!hash.has_value()) return;

			if (hash != vid_res->metadata().sha256)
			{
				debug::error("Selected file has different hash than the original file, can't use it as a replacement");
				return;
			}

			vid_res->set_file_path(result.path.u8string());
			ctx_.is_project_dirty = true;
		});
	}

	void main_window::register_attribute_listeners()
	{
		ctx_.add_event_listener<attribute_add_request_event>([this](const attribute_add_request_event& event)
		{
			auto& tags = ctx_.current_project->tags;
			auto tag_it = tags.find(event.tag_name());
			if (tag_it == tags.end()) return;

			auto& attributes = tag_it->attributes;
			auto attr_it = attributes.find(event.attribute_name());
			if (attr_it != attributes.end()) return;

			auto attr_ptr = ctx_.attr_registry.new_attribute(event.type_name(), event.attribute_name());
			if (attr_ptr == nullptr) return;

			auto [_, inserted] = attributes.try_emplace(event.attribute_name(), std::move(attr_ptr));
			if (!inserted) return;

			ctx_.is_project_dirty = true;
			ctx_.dispatch_event<attribute_added_event>(event_source_, event.tag_name(), event.attribute_name());
		});

		ctx_.add_event_listener<attribute_delete_request_event>([this](const attribute_delete_request_event& event)
		{
			auto& tags = ctx_.current_project->tags;
			auto tag_it = tags.find(event.tag_name());
			if (tag_it == tags.end()) return;

			auto& attributes = tag_it->attributes;
			auto attr_it = attributes.find(event.attribute_name());
			if (attr_it == attributes.end()) return;

			for (auto& [_, group] : ctx_.current_project->video_groups)
			{
				auto& storage = group.segments();
				auto segments_it = storage.find(event.tag_name());
				if (segments_it == storage.end()) continue;

				segments_it->second.erase_attribute_instances(event.attribute_name());
			}

			attributes.erase(attr_it);

			ctx_.is_project_dirty = true;
			ctx_.dispatch_event<attribute_deleted_event>(event_source_, event.tag_name(), event.attribute_name());
		});

		ctx_.add_event_listener<attribute_rename_request_event>([this](const attribute_rename_request_event& event)
		{
			auto& tags = ctx_.current_project->tags;
			auto tag_it = tags.find(event.tag_name());
			if (tag_it == tags.end()) return;

			auto& attributes = tag_it->attributes;
			auto attr_it = attributes.find(event.attribute_name());
			if (attr_it == attributes.end()) return;
			if (attributes.find(event.new_name()) != attributes.end()) return;

			auto node = attributes.extract(attr_it);
			node.key() = event.new_name();
			attributes.insert(std::move(node));

			ctx_.is_project_dirty = true;
			ctx_.dispatch_event<attribute_renamed_event>(event_source_, event.tag_name(), event.attribute_name(), event.new_name());
		});
	}

	void main_window::on_open_project()
	{
		ctx_.main_window->set_subtitle(ctx_.current_project->name);
		ctx_.get_window<widgets::console>().clear();

		for (auto& [video_id, _] : ctx_.current_project->videos)
		{
			if (ctx_.app_settings.load_thumbnails)
			{
				ctx_.dispatch_event<video_load_thumbnail_request_event>("project", video_id, false, true);
			}
		}
	}

	void main_window::on_close_project(bool should_shutdown)
	{
		if (ctx_.script_handle.has_value() and !ctx_.script_handle->has_finished())
		{
			ctx_.script_eng.interrupt();
			return;
		}

		ctx_.session.tasks.cancel_all();
		ctx_.tasks.wait_for_all();

		ctx_.last_focused_video = std::nullopt;

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
			messagebox_data data{};
			data.buttons =
			{
				{ 0, "Save"},
				{ 1, "Don't Save" },
				{ 2, "Cancel" }
			};
			data.icon = messagebox_icon::warning;
			data.title = "VideoTagger";
			data.message = "The current project has unsaved changes.\nDo you want to save pending changes?";
			data.default_button_id = 0;
			data.cancel_button_id = 2;
			data.callback = [this, should_shutdown](int id)
			{
				switch (id)
				{
					case 0: on_save(); break;
					case 1: on_dont_save(); break;
					case 2: return;
				}
				
				if (should_shutdown) 
				{
					on_shutdown();
				}
				else
				{
					ctx_.dispatch_event<video_group_change_request_event>(event_source_, ctx_.get_window<widgets::video_player>(), invalid_video_group_id);
					ctx_.current_project = std::nullopt;
					ctx_.is_project_dirty = false;
					ctx_.session.reset();
					set_subtitle();
				}
			};
			messagebox::show(data);
		}
		else if (should_shutdown)
		{
			on_shutdown();
		}
		else
		{
			ctx_.dispatch_event<video_group_change_request_event>(event_source_, ctx_.get_window<widgets::video_player>(), invalid_video_group_id);
			ctx_.current_project = std::nullopt;
			ctx_.is_project_dirty = false;
			set_subtitle();
			ctx_.session.reset();
		}
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

	void main_window::on_dont_save()
	{

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
		auto& timeline = ctx_.get_window<widgets::timeline>();
		segment_id_map selected_segments = ctx_.session.selected_segments();
		for (const auto& [tag, segments] : selected_segments)
		{
			for (auto& id : segments)
			{
				ctx_.dispatch_event<segment_delete_request_event>(event_source_, ctx_.get_current_segment_storage(), tag, id);
			}
		}
	}

	void main_window::on_launch()
	{
		ctx_.dispatch_event<fetch_themes_event>(event_source_);
		ctx_.dispatch_event<fetch_scripts_event>(event_source_);
	}

	void main_window::on_shutdown()
	{
		ctx_.state_ = app_state::shutdown;
	}

	void main_window::on_first_launch()
	{
		ctx_.reset_layout = true;
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

		load_theme();

		auto& io = ImGui::GetIO();
		if (!std::filesystem::exists(io.IniFilename))
		{
			ctx_.reset_layout = true;
		}
		build_fonts(ctx_.app_settings.font_size);

		ctx_.load_lang_packs(ctx_.app_settings.language.value_or("en_US"));
		return result;
	}

	void main_window::load_theme()
	{
		if (ctx_.app_settings.theme_name.has_value())
		{
			auto theme_name = ctx_.app_settings.theme_name.value();
			debug::log("Loading theme: {}", theme_name);
			auto theme_path = ctx_.theme_dir_filepath / fmt::format("{}.{}", theme_name, theme::extension);
			if (std::filesystem::exists(theme_path))
			{
				ctx_.change_theme(theme::load_from_file(theme_path));
				return;
			}
			else
			{
				debug::error("Failed to load theme, file does not exist: {}", theme_path.u8string());
				debug::log("Resetting preferred theme...");
				ctx_.app_settings.theme_name.reset();
			}
		}

		debug::log("Preferred theme is empty, loading default theme");
		auto default_theme_json = utils::json::from_string(embed::dark_theme);
		ctx_.change_theme(theme::load_from_json(default_theme_json));
		return;
		
	}

	void main_window::load_project(const project_info& project)
	{
		ctx_.current_project = project::load_from_file(project.path);
		on_open_project();
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
		on_close_project(false);
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
		//ctx_.keybinds.insert("Toggle Tag Manager", keybind(SDLK_F7, toggle_window_mod, flags, toggle_window_action("tag-manager", ctx_.win_cfg.show_tag_manager_window)));
		//ctx_.keybinds.insert("Toggle Timeline", keybind(SDLK_F8, toggle_window_mod, flags, toggle_window_action("timeline", ctx_.win_cfg.show_timeline_window)));
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
		ctx_.add_event_listener<tag_deleted_event>([](const tag_deleted_event& event)
		{
			if (!event.deleted()) return;

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

		ctx_.add_event_listener<tag_renamed_event>([](const tag_renamed_event& event)
		{
			if (!event.renamed()) return;

			auto& keybinds = ctx_.current_project->keybinds;
			for (auto& [name, kb] : keybinds)
			{
				auto timeline_action_ptr = std::dynamic_pointer_cast<timeline_action>(kb.action);
				if (timeline_action_ptr == nullptr) continue;
				if (timeline_action_ptr->tag() != event.tag_name()) continue;
				timeline_action_ptr->set_tag(event.new_name());
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
		.add_toggle("Snap to Frame", "Specifies whether to snap the current time to the current frame timestamp", ctx_.app_settings.snap_to_frame)
		.add_toggle("Hardware Acceleration", "Specifies whether to use hardware acceleration for video decoding and processing. Takes effect only for newly opened videos", ctx_.app_settings.hardware_acceleration)
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

#ifdef VT_DEBUG
		.add_label_spacer("Debug Only")
		.add_raw([]()
		{
			auto& io = ImGui::GetIO();

			auto avail_area = ImGui::GetContentRegionAvail();

			static ui::themed_slider<float> font_slider(0.5f, 2.0f, io.FontGlobalScale, { avail_area.x, ImGui::GetFrameHeight() });
			font_slider.set_step(0.005f);
			font_slider.render_with_label("Font Scale");
			font_slider.set_on_change_callback([&io](float old_value, float value)
			{
				io.FontGlobalScale = value;
			});
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

		player.callbacks.on_finish = [](loop_mode mode, bool is_playing)
		{
			
		};

		register_player_listeners();
	}

	utils::file_node main_window::fetch_themes(const std::filesystem::path& path)
	{
		utils::file_node result;
		auto& dark = result["dark"];
		auto& light = result["light"];

		if (!std::filesystem::is_directory(path)) return result;
		for (auto& dir_entry : std::filesystem::directory_iterator(path))
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
							ctx_.dispatch_event<video_open_importer_request_event>(event_source_, importer_id);
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
							if (ImGui::MenuItem("Export Segments", nullptr, nullptr, ctx_.session.current_video_group_id() != invalid_video_group_id))
							{
								const auto& group_name = ctx_.current_project->video_groups.at(ctx_.session.current_video_group_id()).display_name;

								utils::dialog_filter filter{ "VideoTagger Segments", "vtss" };
								auto result = utils::filesystem::save_file({}, { filter }, group_name);
								if (result)
								{
									//TODO: ability to choose which groups to export

									ctx_.current_project->export_segments(result.path, { ctx_.session.current_video_group_id() });

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
					//win_toggles{ "Show Tag Manager", "Toggle Tag Manager", "tag-manager", &ctx_.win_cfg.show_tag_manager_window },
					//win_toggles{ "Show Timeline", "Toggle Timeline", "timeline", &ctx_.win_cfg.show_timeline_window },
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
#ifdef VT_DEBUG
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
								auto filename = path.stem().string();
								auto new_theme = theme::load_from_file(path);
								ctx_.app_settings.theme_name = filename;
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
								auto filename = path.stem().string();
								auto new_theme = theme::load_from_file(path);
								ctx_.app_settings.theme_name = filename;
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
				if (ImGui::MenuItem(ctx_.lang->get("menu_bar.help.check_for_updates").c_str()))
				{
					ctx_.tasks.run([]()
					{
						return update_manager::check_for_updates();
					})
					.then(ctx_.tasks.on_main(), [](const std::optional<update_info>& update)
					{
						if (update.has_value())
						{
							messagebox_data data;
							data.title = ctx_.lang->get("updates.update_available.title");
							data.message = ctx_.lang->get("updates.update_available.message");
							data.default_button_id = 0;
							data.cancel_button_id = 1;
							data.callback = [update](int id)
							{
								if (id == 0)
								{
									update_manager::update(update.value());
								}
							};
							data.icon = messagebox_icon::info;
							data.buttons =
							{
								{ 0, ctx_.lang->get("updates.update") },
								{ 1, ctx_.lang->get("cancel") }
							};
							messagebox::show(data);
						}
						else
						{
							messagebox::show(ctx_.lang->get("updates.no_update.title"), ctx_.lang->get("updates.no_update.message"), messagebox_icon::info);
						}
					});
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

#ifdef VT_DEBUG
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

	void main_window::draw_main_app()
	{
		if (!ctx_.current_project.has_value()) return;
		draw_menubar();
		if (!ctx_.current_project.has_value()) return;

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
			if (ctx_.session.current_video_group_id() == invalid_video_group_id)
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

		ctx_.tag_importer.render(ctx_.win_cfg.show_tag_importer_window);

		if (ctx_.win_cfg.show_options_window)
		{
			ctx_.options.render();
		}
		
		if (ctx_.script_progress_popup != nullptr)
		{
			ctx_.script_progress_popup->open_and_render(!ctx_.script_progress_popup->is_open());
			if (!ctx_.script_progress_popup->is_open())
			{
				ctx_.script_progress_popup.reset();
			}
		}

		if (ctx_.segments_move_conflict_popup != nullptr)
		{
			ctx_.segments_move_conflict_popup->open_and_render(!ctx_.segments_move_conflict_popup->is_open());
			if (!ctx_.segments_move_conflict_popup->is_open())
			{
				ctx_.segments_move_conflict_popup.reset();
			}
		}

		if (ctx_.segment_insert_conflict_popup != nullptr)
		{
			ctx_.segment_insert_conflict_popup->open_and_render(!ctx_.segment_insert_conflict_popup->is_open());
			if (!ctx_.segment_insert_conflict_popup->is_open())
			{
				ctx_.segment_insert_conflict_popup.reset();
			}
		}

		if (ctx_.segment_insert_popup != nullptr)
		{
			ctx_.segment_insert_popup->open_and_render(!ctx_.segment_insert_popup->is_open());
			if (!ctx_.segment_insert_popup->is_open())
			{
				ctx_.segment_insert_popup.reset();
			}
		}

		if (ctx_.tag_rename_failed_popup != nullptr)
		{
			ctx_.tag_rename_failed_popup->open_and_render(!ctx_.tag_rename_failed_popup->is_open());
			if (!ctx_.tag_rename_failed_popup->is_open())
			{
				ctx_.tag_rename_failed_popup.reset();
			}
		}

		if (ctx_.is_video_importer_registered<google_drive_video_importer>())
		{
			auto& importer = ctx_.get_video_importer<google_drive_video_importer>();

			if (importer.open_importer_popup)
			{
				importer.importer_popup.open();
				importer.open_importer_popup = false;
			}

			importer.importer_popup.render();
		}

		auto& timeline = ctx_.get_window<widgets::timeline>();
		bool v = true;
		if (ctx_.session.current_video_group_id() != invalid_video_group_id)
		{
			auto& state = timeline.state();
			state.set_min_timestamp(timestamp::zero());
			state.set_max_timestamp(ctx_.displayed_videos.duration_as_timestamp());
			state.set_current_timestamp(ctx_.displayed_videos.current_timestamp_as_timestamp());
		}

		timeline.set_on_seek_callback([event_source = event_source_](timestamp ts)
		{
			if (ts != ctx_.displayed_videos.current_timestamp_as_timestamp())
			{
				auto& player = ctx_.get_window<widgets::video_player>();
				ctx_.dispatch_event<seek_request_event>(event_source, player, ts.total_milliseconds);
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
			//tag_attribute_instance* selected_attribute = ctx_.get_selected_attribute();
			//bool has_selected_attribute = selected_attribute != nullptr;

			//bool is_shape = has_selected_attribute and selected_attribute->has<shape>() and selected_attribute->get<shape>().get_type() != shape::type::none;
			//if (ctx_.last_focused_video.has_value() and ctx_.displayed_videos.find(ctx_.last_focused_video.value()) == ctx_.displayed_videos.end())
			//{
			//	ctx_.last_focused_video = std::nullopt;
			//	ctx_.set_selected_attribute(nullptr);
			//	ctx_.dispatch_event<gizmo_set_targets_event>(event_source_);
			//}

			//if (!is_shape)
			//{
			//	ctx_.dispatch_event<gizmo_set_targets_event>(event_source_);
			//}

			bool reconfigure = player.prepare_video_windows(ctx_.displayed_videos.size());
			auto& vid_wins = player.video_windows();


			uint64_t vid_id{};
			for (auto& video_data : ctx_.displayed_videos)
			{
				bool timestamp_in_range = video_data.is_timestamp_in_range(ctx_.displayed_videos.current_timestamp());

				//TODO: handle is_widget_open
				bool is_widget_open = true;
				ImVec2 point_pos{};
				ImVec2 start_pos{};

				bool has_target = ctx_.session.has_gizmo_targets();
				
				if (has_target)
				{
					auto mean_point = ctx_.session.mean_gizmo_target();
					point_pos = { (float)mean_point.at(0), (float)mean_point.at(1) };
					start_pos = point_pos;
				}

				//widgets::draw_video_widget(video_data.video, video_data.display_texture, timestamp_in_range, is_widget_open, vid_id++,
				auto video_ptr = ctx_.current_project->videos.get(video_data.id);
				if (video_ptr == nullptr) continue;

				auto video_name = video_ptr->title();

				auto& vid_win = vid_wins[vid_id];
				vid_win->set_active(timestamp_in_range);
				
				if (reconfigure)
				{
					vid_win->set_display_name(video_name);
					vid_win->set_video(video_data.video, video_data.id);
					vid_win->set_texture(video_data.display_texture);
				}

				if (true)
				{
					//TODO: Overlays shouldn't be cleared every frame, rewrite the overlay to not copy values - it should get them by itself
					vid_win->clear_overlays();
					//Segment Attribute Instance Overlays
					vid_win->with_overlay([source = vid_win->get_event_source()](video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
					{
						auto current_ts = ctx_.displayed_videos.current_timestamp_as_timestamp();

						auto& segment_storage = ctx_.get_current_segment_storage();
						for (auto& [tag_name, segments] : segment_storage)
						{
							auto& tag = ctx_.current_project->tags.at(tag_name);

							auto segment_it = segments.find(current_ts);
							if (segment_it == segments.end()) continue;

							auto& segment_attr_instances = segments.segment_attribute_instances(segment_it->id);
							auto video_attr_it = segment_attr_instances.find(video_id);
							if (video_attr_it == segment_attr_instances.end()) continue;

							auto& video_attr_instances = video_attr_it->second;
							for (auto& attr_instance_ptr : video_attr_instances)
							{
								if (attr_instance_ptr == nullptr) continue;

								attr_instance_ptr->render_overlay(tag, segment_it->id, current_ts, video_id, pos, size, tex_size);
							}
						}

						bool window_hovered = ImGui::IsWindowHovered();
						bool select_tool_active = ctx_.session.toolbar.is_tool_active("select");
						const auto& hovered_regions = ctx_.session.hovered_regions();
						bool is_over_gizmo = ImGuizmo::IsOver() and ctx_.session.has_gizmo_targets();

						if (window_hovered and select_tool_active and !is_over_gizmo)
						{
							if (!hovered_regions.empty())
							{
								auto& region_data = hovered_regions.front();

								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
								ui::tooltip(fmt::format("Tag: {}\nAttribute: {}", region_data.tag_name, region_data.attribute_instance->attribute_name()));
							}

							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
							{
								if (!hovered_regions.empty())
								{
									auto& region_data = hovered_regions.front();
									ctx_.dispatch_event<region_select_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id, *region_data.attribute_instance, region_data.region_id);
								}
								else if (ctx_.session.is_any_region_selected())
								{
									ctx_.dispatch_event<region_deselect_request_event>(source);
									ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, std::vector<utils::vec2<uint32_t>*>{});
								}
							}
						}
					});

					//Tool overlays
					vid_win->with_overlay([](video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
					{
						for (auto& [id, group] : ctx_.session.toolbar.groups())
						{
							auto* entry = ctx_.session.toolbar.active_entry();
							if (entry == nullptr) continue;

							auto* active_tool = entry->active_tool();
							if (active_tool == nullptr) continue;

							active_tool->render_overlay(video_id, pos, size, tex_size);
						}
					});

					//Video ID Overlay
					vid_win->with_overlay([vid_id](video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
					{
						auto& player = ctx_.get_window<widgets::video_player>();
						if (!player.show_video_ids()) return;

						static constexpr float win_padding = 10.f;
						static constexpr float text_padding = 10.f;

						const auto& style = ImGui::GetStyle();
						const auto& theme = ctx_.current_theme;
						auto draw_list = ImGui::GetWindowDrawList();
						auto win_rect = ImGui::GetCurrentWindow()->InnerRect;

						auto cpos = ImGui::GetCursorPos();
						ImGui::PushFont(ctx_.get_font(font_type::h1));
						std::string text = std::to_string(vid_id + 1);
						ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
						float text_side = std::max(text_size.x, text_size.y);
						text_side += 2 * text_padding;

						ImRect bg_rect = ImRect{ win_rect.Min.x, win_rect.Max.y - text_side, win_rect.Min.x + text_side, win_rect.Max.y };
						bg_rect.Translate({ win_padding, -win_padding });
						auto text_pos = ImVec2{ bg_rect.Min.x + (bg_rect.GetWidth() - text_size.x) * 0.5f, bg_rect.Min.y + text_padding };

						auto color = theme.get_float4(theme_color::text_inverted);
						color.w *= 0.75f;
						auto color_u32 = ImGui::GetColorU32(color);
						draw_list->AddRectFilled(bg_rect.Min, bg_rect.Max, color_u32, 7.f);
						ImGui::RenderText(text_pos, text.c_str());
						ImGui::PopFont();
					});
					
					vid_win->with_overlay([source = vid_win->get_event_source(), has_target, &point_pos, &start_pos](video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size)
					{
						bool select_tool_active = ctx_.session.toolbar.is_tool_active("select");
						if (!select_tool_active or ctx_.session.gizmo_video_id() != video_id) return;

						static auto from_pixels = [&tex_size, &size](uint32_t value) -> float
						{
							float viewport_diagonal = utils::intersection::length(size);
							float tex_diagonal = utils::intersection::length(tex_size);
							return (float)value * viewport_diagonal / tex_diagonal;
						};

						if (has_target)
						{
							float left = 0.0f;
							float right = tex_size.x;
							float bottom = tex_size.y;
							float top = 0.f;
							float near_z = -1.0f;
							float far_z = 1.0f;

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

								utils::vec2<uint32_t> offset{ (uint32_t)(point_pos.x - start_pos.x), (uint32_t)(point_pos.y - start_pos.y) };
								ctx_.dispatch_event<gizmo_move_targets_event>(source, video_id, ctx_.session.gizmo_targets(), offset);
							}
						}
					});

					//vid_win->with_overlay([&point_pos, start_pos, has_selected_attribute, selected_attribute, is_shape, has_target, &video_data, this](ImVec2 pos, ImVec2 size, ImVec2 tex_size)
					//{
					//	static auto from_tex_pos = [&pos, &tex_size, &size](const ImVec2 point) -> ImVec2
					//	{
					//		return pos + (point / tex_size) * size;
					//	};

					//	static auto to_tex_pos = [&pos, &tex_size, &size](const ImVec2 point) -> utils::vec2<uint32_t>
					//	{
					//		ImVec2 tex_coords = (point - pos) / size * tex_size;
					//		return utils::vec2<uint32_t>{ static_cast<uint32_t>(std::round(tex_coords.x)), static_cast<uint32_t>(std::round(tex_coords.y)) };
					//	};

					//	static auto from_pixels = [&tex_size, &size](uint32_t value) -> float
					//	{
					//		float viewport_diagonal = utils::intersection::length(size);
					//		float tex_diagonal = utils::intersection::length(tex_size);
					//		return (float)value * viewport_diagonal / tex_diagonal;
					//	};

					//	static auto to_pixels = [&tex_size, &size](float value) -> uint32_t
					//	{
					//		float viewport_diagonal = utils::intersection::length(size);
					//		float tex_diagonal = utils::intersection::length(tex_size);
					//		return (uint32_t)(value * tex_diagonal / viewport_diagonal);
					//	};

					//	ImGuiIO& io = ImGui::GetIO();
					//	bool hovered = ImGui::IsWindowHovered();
					//	bool focused = ImGui::IsWindowFocused();
					//	auto border_color = hovered ? 0xFF00FF00 : 0xFF0000FF;
					//	float border_thickness = 2.0f;

					//	if (focused)
					//	{
					//		ctx_.last_focused_video = video_data.id;
					//	}

					//	bool last_focused = ctx_.last_focused_video == video_data.id;

					//	bool is_polygon = is_shape and selected_attribute->get<shape>().get_type() == shape::type::polygon;

					//	ImVec2 add_point_pos{};
					//	bool add_point{};

					//	auto current_ts = ctx_.displayed_videos.current_timestamp_as_timestamp();
					//	bool can_add_point{};

					//	bool is_keyframe{};
					//	if (is_shape)
					//	{
					//		const auto& shape = selected_attribute->get<vt::shape>();
					//		shape.visit([current_ts, &is_keyframe](const auto& map)
					//		{
					//			if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
					//			{
					//				auto it = map.find(current_ts);
					//				if (it != map.end())
					//				{
					//					is_keyframe = true;
					//				}
					//			}
					//		});
					//	}

					//	if (is_polygon)
					//	{
					//		const auto& shape = selected_attribute->get<vt::shape>();
					//		can_add_point = is_keyframe;
					//	}

					//	if (last_focused and is_shape and ImGui::BeginPopupContextItem("##VideoCtxMenu"))
					//	{
					//		auto& shape = selected_attribute->get<vt::shape>();
					//		bool close = false;
					//		const auto& style = ImGui::GetStyle();

					//		if (can_add_point and ImGui::MenuItem("Add Point", nullptr, nullptr))
					//		{
					//			add_point_pos = ImGui::GetWindowPos();
					//			add_point = true;
					//		}

					//		if (ImGui::MenuItem(fmt::format("{} Add Keyframe", icons::add_keyframe).c_str(), nullptr, nullptr, !is_keyframe))
					//		{
					//			shape.visit([current_ts, &is_keyframe, &shape](auto& map)
					//			{
					//				if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
					//				{
					//					auto it = map.lower_bound(current_ts);
					//					if (map.empty())
					//					{
					//						map[current_ts].push_back({});
					//					}
					//					else
					//					{
					//						if (it != map.begin() and (it == map.end() or it->first != current_ts))
					//						{
					//							--it;
					//						}
					//						map[current_ts] = it->second;
					//					}
					//					is_keyframe = true;
					//					ctx_.is_project_dirty = true;
					//				}
					//			});
					//		}

					//		if (ImGui::MenuItem(fmt::format("{} Add Region", shape.type_icon(shape.get_type())).c_str(), nullptr, nullptr, is_keyframe))
					//		{
					//			shape.visit([current_ts, &is_keyframe, this](auto& map)
					//			{
					//				if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(map)>>>)
					//				{
					//					auto& keyframe = map.at(current_ts);
					//					keyframe.push_back({});
					//					keyframe.back().set_target(event_source_);
					//					ctx_.is_project_dirty = true;
					//				}
					//			});
					//		}

					//		if (ImGui::BeginMenu("Transform", has_target))
					//		{
					//			auto icon_size = ImGui::CalcTextSize(icons::align_center).x;
					//			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
					//			if (ImGui::BeginTable("##AlignTable", 3, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInner, { 3.f * (icon_size + 2 * style.FramePadding.x), 0.f }))
					//			{
					//				ImGui::TableNextRow();
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_horizontal_center))
					//				{
					//					point_pos.x = tex_size.x / 2;
					//					close = true;
					//				}
					//				ui::tooltip("Align Horizontal Center");
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_vertical_top))
					//				{
					//					point_pos.y = 0;
					//					close = true;
					//				}
					//				ui::tooltip("Align Vertical Top");

					//				ImGui::TableNextRow();
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_horizontal_left))
					//				{
					//					point_pos.x = 0;
					//					close = true;
					//				}
					//				ui::tooltip("Align Horizontal Left");
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_center))
					//				{
					//					point_pos = tex_size / 2;
					//					close = true;
					//				}
					//				ui::tooltip("Align Center");
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_horizontal_right))
					//				{
					//					point_pos.x = tex_size.x;
					//					close = true;
					//				}
					//				ui::tooltip("Align Horizontal Right");

					//				ImGui::TableNextRow();
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_vertical_center))
					//				{
					//					point_pos.y = tex_size.y / 2;
					//					close = true;
					//				}
					//				ui::tooltip("Align Vertical Center");
					//				ImGui::TableNextColumn();
					//				if (ui::icon_button(icons::align_vertical_bottom))
					//				{
					//					point_pos.y = tex_size.y;
					//					close = true;
					//				}
					//				ui::tooltip("Align Vertical Bottom");

					//				ImGui::EndTable();
					//			}
					//			ImGui::PopStyleVar();
					//			ImGui::EndMenu();
					//		}

					//		if (close)
					//		{
					//			utils::vec2<uint32_t> offset = { (uint32_t)(point_pos.x - start_pos.x), (uint32_t)(point_pos.y - start_pos.y) };
					//			ctx_.dispatch_event<gizmo_move_targets_event>(event_source_, ctx_.session.gizmo_targets(), offset);
					//			ImGui::CloseCurrentPopup();
					//		}
					//		ImGui::EndPopup();
					//	}

					//	if (!add_point and can_add_point and (ImGui::IsKeyDown(ImGuiKey_LeftShift) or ImGui::IsKeyDown(ImGuiKey_RightShift)) and ImGui::IsMouseClicked(ImGuiMouseButton_Left) and hovered)
					//	{
					//		add_point_pos = ImGui::GetMousePos();
					//		add_point = true;
					//	}
					//	else if (is_shape and !add_point and ImGui::IsMouseClicked(ImGuiMouseButton_Left) and hovered)
					//	{
					//		auto& shape = selected_attribute->get<vt::shape>();
					//		auto closest_target = shape.closest_point(current_ts, to_tex_pos(ImGui::GetMousePos()), from_pixels(10));
					//		if (closest_target != nullptr)
					//		{
					//			ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, { { closest_target } });
					//		}
					//	}

					//	if (add_point and can_add_point and is_polygon)
					//	{
					//		auto& shape = selected_attribute->get<vt::shape>();
					//		auto& map = shape.get_map<polygon>();
					//		auto& polygons = map.at(current_ts); //this keyframe definitely exists, it was checked before

					//		auto it = std::find_if(polygons.begin(), polygons.end(), [](const polygon& poly)
					//		{
					//			for (const auto& vertex : poly.vertices)
					//			{
					//				if (ctx_.session.gizmo_contains_target(&vertex)) return true;
					//			}
					//			return false;
					//		});

					//		bool all_empty = true;
					//		for (const auto& poly : polygons)
					//		{
					//			if (!poly.vertices.empty())
					//			{
					//				all_empty = false;
					//				break;
					//			}
					//		}

					//		if (all_empty)
					//		{
					//			polygons.front().vertices.push_back({ to_tex_pos(add_point_pos) });
					//		}
					//		else if (it == polygons.end())
					//		{
					//			//add new polygon with that point
					//			polygons.push_back(polygon{ { to_tex_pos(add_point_pos) } });
					//		}
					//		else
					//		{
					//			auto& polygon = *it;
					//			auto& pos = add_point_pos;

					//			auto closest_it = polygon.vertices.end();
					//			float min_distance = std::numeric_limits<float>::infinity();

					//			for (auto it = polygon.vertices.begin(); it != polygon.vertices.end(); ++it)
					//			{
					//				auto next_it = std::next(it);
					//				if (next_it == polygon.vertices.end())
					//				{
					//					next_it = polygon.vertices.begin();
					//				}

					//				const auto& vertex1 = *it;
					//				const auto& vertex2 = *next_it;

					//				float new_distance = utils::intersection::distance_to_segment(pos, from_tex_pos({ (float)vertex1[0], (float)vertex1[1] }), from_tex_pos({ (float)vertex2[0], (float)vertex2[1] }));


					//				if (new_distance < min_distance)
					//				{
					//					min_distance = new_distance;
					//					closest_it = it;
					//				}
					//			}

					//			if (closest_it != polygon.vertices.end())
					//			{
					//				auto next_it = std::next(closest_it);
					//				if (next_it == polygon.vertices.end())
					//				{
					//					next_it = polygon.vertices.begin();
					//				}

					//				auto new_target = &*polygon.vertices.insert(next_it, to_tex_pos(pos));
					//				ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, { { new_target } });
					//			}
					//			else
					//			{
					//				auto new_target = &*polygon.vertices.insert(closest_it, to_tex_pos(pos));
					//				ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, { { new_target } });
					//			}
					//		}
					//	}

					//	auto draw_list = ImGui::GetWindowDrawList();
					//	//draw_list->AddRectFilled(top_left, bottom_right, overlay_color);
					//	/*draw_list->AddLine(top_left, bottom_right, border_color, border_thickness);
					//	draw_list->AddLine(top_right, bottom_left, border_color, border_thickness);*/

					//	ImVec2 top_left = { pos.x, pos.y };
					//	ImVec2 top_right = { pos.x + size.x, pos.y };
					//	ImVec2 bottom_left = { pos.x, pos.y + size.y };
					//	ImVec2 bottom_right = { pos.x + size.x, pos.y + size.y };

					//	float left = 0.0f;
					//	float right = tex_size.x;
					//	float bottom = tex_size.y;
					//	float top = 0.f;
					//	float near_z = -1.0f;
					//	float far_z = 1.0f;

					//	//shape drawing
					//	std::string tooltip;
					//	for (const auto& displayed_tag : ctx_.current_project->displayed_tags)
					//	{
					//		auto& segment_storage = ctx_.get_current_segment_storage();
					//		auto it = segment_storage.find(displayed_tag);
					//		if (it == segment_storage.end()) continue;
					//		auto& tag = ctx_.current_project->tags.at(it->first);
					//		auto fill_color = (tag.color & ~0xFF000000) | 0x80000000;

					//		auto& segments = it->second;
					//		for (auto& [segment_id, segment] : segments)
					//		{
					//			bool is_onscreen = current_ts >= segment.start and current_ts <= segment.end;
					//			if (is_onscreen)
					//			{
					//				auto segment_attr_it = segment.attributes.find(video_data.id);
					//				if (segment_attr_it != segment.attributes.end())
					//				{
					//					for (auto& [attr_name, attr] : segment_attr_it->second)
					//					{
					//						if (!attr.has<shape>()) continue;

					//						bool is_selected = selected_attribute == &attr;
					//						bool show_points = is_selected and is_keyframe;

					//						auto& shape = attr.get<vt::shape>();
					//						draw_list->PushClipRect(top_left, bottom_right, true);

					//						shape.draw(current_ts, !is_keyframe and shape.interpolate, from_tex_pos, from_pixels, tex_size, size, is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : tag.color, fill_color, show_points, [&, this](size_t i, const std::vector<utils::vec2<uint32_t>*>& vertices)
					//						{
					//							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and hovered)
					//							{
					//								ctx_.dispatch_event<segment_deselect_all_request_event>(event_source_, segment_storage);
					//								ctx_.dispatch_event<segment_select_request_event>(event_source_, segment_storage, tag.name, segment_id);
					//								ctx_.set_selected_attribute(&attr);

					//								if (is_keyframe and !vertices.empty() and !ImGuizmo::IsOver())
					//								{
					//									ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, vertices);
					//								}
					//							}
					//							tooltip = fmt::format("Tag: {}\nAttribute: {}\nID: {}", tag.name, attr_name, i + 1);
					//						});
					//					}
					//				}
					//			}
					//		}
					//	}

					//	if (hovered and !tooltip.empty() and !ImGuizmo::IsOver())
					//	{
					//		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
					//		ui::tooltip(tooltip);
					//	}

					//	if (!is_keyframe and has_target)
					//	{
					//		ctx_.dispatch_event<gizmo_set_targets_event>(event_source_);
					//	}

					//	if (last_focused and has_target and is_keyframe)
					//	{
					//		auto wpos = ImGui::GetWindowPos();
					//		auto wsize = ImGui::GetWindowSize();

					//		float translation[3]{ point_pos.x, point_pos.y, 0.0f };
					//		float rotation[3]{};
					//		float scale[3] = { 1.f, 1.f, 1.f };

					//		float target[3]
					//		{
					//			utils::matrix::front[0], //translation[0] + utils::matrix::front[0],
					//			utils::matrix::front[1], //translation[1] + utils::matrix::front[1],
					//			utils::matrix::front[2], //translation[2] + utils::matrix::front[2]
					//		};

					//		float cam_distance = 0.f;
					//		float cam_angle[2]{};
					//		float eye[3]
					//		{
					//			std::cos(cam_angle[1]) * std::cos(cam_angle[0]) * cam_distance,
					//			std::sin(cam_angle[0]) * cam_distance,
					//			std::sin(cam_angle[1]) * std::cos(cam_angle[0]) * cam_distance
					//		};
					//		utils::matrix view_mat = (utils::matrix::look_at(eye, target));

					//		utils::matrix proj_mat = utils::matrix::ortho(left, right, bottom, top, near_z, far_z);
					//		ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

					//		utils::matrix mod{};
					//		auto& gizmo_style = ImGuizmo::GetStyle();

					//		gizmo_style.CenterCircleSize = ctx_.app_settings.scale_gizmos ? from_pixels(5) : 5.f;
					//		gizmo_style.ScaleLineCircleSize = gizmo_style.CenterCircleSize;
					//		gizmo_style.TranslationLineThickness = 2.f * gizmo_style.CenterCircleSize / 3.f;
					//		gizmo_style.TranslationLineArrowSize = 1.5f * gizmo_style.TranslationLineThickness;
					//		ImGuizmo::SetOrthographic(true);
					//		ImGuizmo::SetDrawlist();

					//		float snap[3]{ 1.00f, 1.00f, 1.00f };
					//		ImVec2 obj_size{ 5, 100.f };
					//		float bounds[] = { -obj_size.y / 2, -obj_size.x / 2, 0.f, obj_size.y / 2, obj_size.x / 2, 0.f };
					//		ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, mod.data);
					//		if (ImGuizmo::Manipulate(view_mat.data, proj_mat.data, ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y/* | ImGuizmo::OPERATION::BOUNDS*/, ImGuizmo::MODE::LOCAL, mod.data, nullptr, snap, nullptr/*bounds*/))
					//		{
					//			ImGuizmo::DecomposeMatrixToComponents(mod.data, translation, rotation, scale);
					//			point_pos.x = std::clamp(translation[0], 0.0f, tex_size.x);
					//			point_pos.y = std::clamp(translation[1], 0.0f, tex_size.y);

					//			utils::vec2<uint32_t> offset{ (uint32_t)(point_pos.x - start_pos.x), (uint32_t)(point_pos.y - start_pos.y) };
					//			ctx_.dispatch_event<gizmo_move_targets_event>(event_source_, ctx_.session.gizmo_targets(), offset);
					//		}
					//	}

					//	//window focus frame
					//	if (ctx_.session.is_any_segment_selected() and last_focused and ctx_.last_focused_video.has_value())
					//	{
					//		draw_list->AddRect(top_left, bottom_right, ctx_.current_theme.get_rgba(theme_color::selection_normal), 0, 0, border_thickness);
					//	}
					//	//auto local_pos = from_tex_pos(point_pos);
					//	//draw_list->AddCircle(local_pos, 10.f, border_color);
					//});
				}

				++vid_id;
				vid_win->open_and_render();
			}
		}

		if (ctx_.reset_player_docking)
		{
			auto it = ctx_.current_project->video_groups.find(ctx_.session.current_video_group_id());
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
			ImGui::DockBuilderDockWindow(ctx_.get_window<ui::windows::tag_manager>().name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(ctx_.get_window<ui::windows::region_properties>().name().c_str(), main_dock_right);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_group_queue>().name().c_str(), main_dock_down);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_player>().name().c_str(), main_dock_up);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::localization_editor>().name().c_str(), main_dock_up);
#ifdef VT_DEBUG
			ImGui::DockBuilderDockWindow(ctx_.get_window<ui::windows::sandbox>().name().c_str(), main_dock_up);
#endif
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_browser>().name().c_str(), main_dock_up_left);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::theme_customizer>().name().c_str(), main_dock_up);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::console>().name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::timeline>().name().c_str(), dockspace_id_copy);
			ImGui::DockBuilderDockWindow(ctx_.get_window<widgets::video_group_browser>().name().c_str(), dockspace_id_copy);

			auto queue_node = ImGui::DockBuilderGetNode(main_dock_down);
			queue_node->LocalFlags = ImGuiDockNodeFlags_NoResizeY;

			ImGui::DockBuilderFinish(dockspace_id);
			ctx_.reset_layout = false;
		}

		ImGui::DockSpace(dockspace_id, ImVec2{}, dock_flags);
		ctx_.current_project.has_value() ? draw_main_app() : draw_project_selector();

		ImGui::End();
		ctx_.render_messagebox();
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
					SDL_free(event.drop.file);
				}
			}
			break;
		}
	}
}
