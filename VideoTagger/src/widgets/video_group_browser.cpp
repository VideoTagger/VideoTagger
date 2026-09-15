#include "pch.hpp"
#include "video_group_browser.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <utils/thumbnail.hpp>
#include <utils/string.hpp>
#include "modal/create_group_popup.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/group_video_tile.hpp>

#include "controls.hpp"
#include <events/video_group/video_group_remove_video_event.hpp>
#include <events/video_group/video_open_properties_request_event.hpp>
#include <events/video_group/video_change_offset_request_event.hpp>
#include <events/video_group/video_change_offset_event.hpp>

namespace vt::widgets
{
	static constexpr ImVec2 tile_size{ 65.f, 105.f };

	video_group_browser::video_group_browser() :
		ui::window{ "Video Group Browser", "video-group-browser", "Video Group Browser", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse },
		video_properties_popup_{ std::move(ui::new_popup<ui::video_properties_popup>(get_event_source(), &open_properties_)) }, open_properties_{}
	{
		set_icon(icons::browser);
		register_listeners();
	}

	void video_group_browser::on_open_video(video_id_t video_id)
	{
		//auto& vid_resource = ctx_.current_project->videos.get(id);
		ctx_.reset_player_docking = true;
		//ctx_.current_project->videos.open_video(id);
	}

	void video_group_browser::on_render()
    {
		static auto group_ctx_menu = [](bool& open, bool& remove, bool& enqueue, bool can_enqueue)
		{
			if (ImGui::MenuItem("Add to queue", nullptr, nullptr, can_enqueue))
			{
				enqueue = true;
			}
			if (ImGui::MenuItem("Open"))
			{
				open = true;
			}
			if (ImGui::MenuItem("Remove"))
			{
				remove = true;
			}
		};

		static auto draw_group_tile = [this](video_group& vgroup, video_group_id_t gid, ImVec2 tile_size, bool& open, bool& remove, bool& enqueue, bool can_enqueue)
		{
			const auto& theme = ctx_.current_theme;

			ImGui::PushID((void*)gid);

			auto image = utils::thumbnail::font_texture();
			auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_group_icon);

			ImVec4 tint_color = theme.get_float4(theme_color::icon_thumbnail);

			open |= widgets::tile(fmt::format("group{}", gid).c_str(), vgroup.display_name, tile_size, tile_size, image,
			[&](const std::string& label)
			{
				group_ctx_menu(open, remove, enqueue, can_enqueue);
			},
			[&](const std::string& label)
			{
				if (ImGui::BeginDragDropTarget())
				{
					auto payload = utils::drag_drop::get_payload<video_id_t>("Video", ImGuiDragDropFlags_AcceptBeforeDelivery);
					if (payload.data.has_value())
					{
						auto is_delivery = payload.imgui_payload->IsDelivery();

						video_group::video_info vinfo;
						vinfo.id = *payload.data;

						bool already_contains = vgroup.contains(vinfo.id);

						if (!is_delivery and already_contains)
						{
							ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
						}
						else if (is_delivery and !already_contains)
						{
							vgroup.insert(vinfo);
							ctx_.is_project_dirty = true;
							debug::log("Added video with id: {} to group with id: {}", vinfo.id, current_video_group);
						}
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
				{
					utils::drag_drop::set_payload("Group", gid);
					std::string str = fmt::format("{} {}", icons::video_group, vgroup.display_name);
					ImGui::TextUnformatted(str.c_str());
					ImGui::EndDragDropSource();
				}
			},
			nullptr, glyph.uv0, glyph.uv1, false, tint_color);
			ImGui::PopID();
		};

		auto& style = ImGui::GetStyle();
		bool open_create_group_popup{};
		//TODO: This should be inside the payload, not here
		static std::vector<video_id_t> dragged_videos;

		//TODO: remove this if
		if (true or ctx_.current_project->videos.size() > 0)
		{
			ImVec2 img_tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
			ImVec2 old_tile_size = img_tile_size + style.ItemSpacing + style.CellPadding / 2;
			auto avail = ImGui::GetContentRegionAvail() - ImVec2{ 0, ImGui::GetTextLineHeightWithSpacing() };
			avail.x *= 0.80f;

			int columns = static_cast<int>(avail.x / tile_size.x) - 1; //TODO: Why -1 has to be here?
			if (columns < 1)
			{
				columns = 1;
			}

			static auto draw_group_tab = [&style, this](const std::string& group_name, video_group_id_t gid, bool& open, bool& remove, bool& enqueue, bool can_enqueue)
			{
				bool inactive = current_video_group != gid;

				if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
				if (ImGui::TreeNodeEx(group_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf) and ImGui::IsItemClicked())
				{
					current_video_group = gid;
				}
				if (inactive) ImGui::PopStyleColor();

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
				{
					utils::drag_drop::set_payload("Group", gid);
					std::string str = fmt::format("{} {}", icons::video_group, group_name);
					ImGui::TextUnformatted(str.c_str());
					ImGui::EndDragDropSource();
				}

				if (gid != 0)
				{
					//TODO: This is duplicated in 2 places
					if (ImGui::BeginDragDropTarget())
					{
						auto payload = utils::drag_drop::get_payload<video_id_t>("Video", ImGuiDragDropFlags_AcceptBeforeDelivery);
						if (payload.data.has_value())
						{
							bool is_delivery = payload.imgui_payload->IsDelivery();

							video_group::video_info vinfo;
							vinfo.id = *payload.data;
							auto& groups = ctx_.current_project->video_groups;
							auto it = groups.find(gid);

							bool is_valid = it != groups.end() and !it->second.contains(vinfo.id);
							if (!is_delivery and !is_valid)
							{
								ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
							}
							else if (is_delivery and is_valid)
							{
								it->second.insert(vinfo);
								ctx_.is_project_dirty = true;
								debug::log("Added video with id: {} to group with id: {}", vinfo.id, current_video_group);
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (ImGui::BeginPopupContextItem())
					{
						group_ctx_menu(open, remove, enqueue, can_enqueue);
						ImGui::EndPopup();
					}
				}
			};

			if (ImGui::BeginTable("##VideoGroupBrowser", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn(nullptr, 0, 0.20f);
				ImGui::TableSetupColumn(nullptr, 0, 0.80f);

				ImGui::TableNextColumn();

				ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(icons::add).x);
				if (ui::icon_button(icons::add))
				{
					open_create_group_popup = true;
				}
				ImGui::TableNextColumn();

				static std::string filter;
				if (ImGui::IsWindowAppearing())
				{
					filter.clear();
				}

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
				bool can_back_out = current_video_group != 0;
				if (!can_back_out) ImGui::BeginDisabled();
				if (ui::icon_button(icons::back))
				{
					current_video_group = 0;
				}
				if (!can_back_out) ImGui::EndDisabled();
				ImGui::SameLine();
				search_bar("##VideoGroupBrowserSearch", ctx_.lang->get("search_hint").c_str(), filter);
				ImGui::PopStyleVar();

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				bool open{};
				bool remove{};
				bool enqueue{};

				draw_group_tab("All Groups", 0, open, remove, enqueue, false);
				ImGui::Separator();

				if (ImGui::BeginChild("##VideoBrowserGroupTabs"))
				{
					auto& playlist = ctx_.current_project->video_group_playlist;

					for (const auto& [gid, group] : ctx_.current_project->video_groups)
					{
						open = false;
						remove = false;
						enqueue = false;
						bool can_enqueue = std::find(playlist.begin(), playlist.end(), gid) == playlist.end();

						std::string group_name = group.display_name;

						draw_group_tab(group_name, gid, open, remove, enqueue, can_enqueue);

						//TODO: Refactor this so this isn't duplicated in 2 places
						if (remove)
						{
							if (current_video_group == gid)
							{
								current_video_group = invalid_video_group_id;
							}
							ctx_.current_project->remove_video_group(gid);
							break;
						}

						if (open)
						{
							debug::log("Opening group {}", gid);
							current_video_group = gid;
							ctx_.reset_player_docking = true;
						}

						if (enqueue)
						{
							playlist.push_back(gid);

							//TODO: Remove when queue gets fully implemented
							/*auto& pool = ctx_.current_project->videos;
							ctx_.current_video_group_id = gid;

							for (auto& vinfo : group)
							{
								auto metadata = pool.get(vinfo.id);
								if (metadata == nullptr) continue;

								if (!metadata->is_widget_open)
								{
									debug::log("Opening video {}", metadata->path.u8string());
									on_open_video(vinfo.id);
								}
							}*/
						}
					}

					//TODO: This is also duplicated below
					ImRect inner_rect = ImGui::GetCurrentWindow()->InnerRect;
					if (ImGui::BeginDragDropTargetCustom(inner_rect, ImGui::GetID("TabsDragDropPanel")))
					{
						auto payload = utils::drag_drop::get_payload<video_id_t>("Video");
						if (payload.data.has_value())
						{
							video_group::video_info vinfo;
							vinfo.id = *payload.data;

							dragged_videos.push_back(vinfo.id);
							open_create_group_popup = true;
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::EndChild();

				ImGui::TableNextColumn();
				size_t filter_passes{};

				auto table_start_cpos = ImGui::GetCursorPos();
				if (ImGui::BeginTable("##VideoBrowserBody", columns, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame, ImGui::GetContentRegionAvail()))
				{
					std::vector<std::string> tokens;
					if (!filter.empty())
					{
						tokens = utils::string::split(utils::string::to_lowercase(utils::string::trim_whitespace(filter)), ' ');
					}

					ImGui::TableNextRow();
					if (current_video_group == invalid_video_group_id)
					{
						auto& playlist = ctx_.current_project->video_group_playlist;

						for (auto& [gid, group] : ctx_.current_project->video_groups)
						{
							bool open_group = false;
							bool remove_group = false;
							bool enqueue_group = false;
							bool can_enqueue = std::find(playlist.begin(), playlist.end(), gid) == playlist.end();

							//filtering
							{
								bool passes_filter = true;
								for (const auto& token : tokens)
								{
									auto ttoken = utils::string::trim_whitespace(token);
									std::string name = utils::string::to_lowercase(group.display_name);
									passes_filter &= name.find(ttoken) != std::string::npos;
								}

								if (!passes_filter) continue;
								++filter_passes;
							}

							ImGui::TableNextColumn();
							draw_group_tile(group, gid, old_tile_size, open_group, remove_group, enqueue_group, can_enqueue);
							if (remove_group)
							{
								if (current_video_group == gid)
								{
									current_video_group = invalid_video_group_id;
								}
								ctx_.current_project->remove_video_group(gid);
								break;
							}

							if (open_group)
							{
								debug::log("Opening group {}", gid);
								current_video_group = gid;
								ctx_.reset_player_docking = true;
							}

							if (enqueue_group)
							{
								playlist.push_back(gid);

								//TODO: This plays the group, remove when queue gets fully implemented
								/*
								auto& pool = ctx_.current_project->videos;
								ctx_.current_video_group_id = gid;

								for (auto& vinfo : group)
								{
									auto metadata = pool.get(vinfo.id);
									if (metadata == nullptr) continue;

									if (!metadata->is_widget_open)
									{
										debug::log("Opening video {}", metadata->path.u8string());
										on_open_video(vinfo.id);
									}
								}
								*/
							}
						}
					}
					else
					{
						auto& pool = ctx_.current_project->videos;
						auto& vgroup = ctx_.current_project->video_groups.at(current_video_group);
						for (auto& vinfo : vgroup)
						{
							auto vid_resource = pool.get(vinfo.id);

							//filtering
							{
								bool passes_filter = true;
								for (const auto& token : tokens)
								{
									auto ttoken = utils::string::trim_whitespace(token);
									std::string name = utils::string::to_lowercase(vid_resource->metadata().title.value_or(""));
									passes_filter &= name.find(ttoken) != std::string::npos;
								}

								if (!passes_filter) continue;
								++filter_passes;
							}

							ImGui::TableNextColumn();
							ui::group_video_tile tile{ *vid_resource, current_video_group, tile_size };
							tile.render();

							/*
							if (open_video and !metadata->is_widget_open)
							{
								debug::log("Opening video {}", metadata->path.u8string());
								on_open_video(vinfo.id);
							}
							*/

							if (ImGui::IsWindowHovered() and ImGui::IsKeyPressed(ImGuiKey_Escape))
							{
								current_video_group = 0;
							}
						}
					}
					ImGui::EndTable();
				}

				auto table = ImGui::GetCurrentTable();
				if (table != nullptr)
				{
					ImRect inner_rect = ImGui::TableGetCellBgRect(table, table->CurrentColumn);
					if (filter_passes == 0)
					{
						bool is_group_view = (current_video_group == invalid_video_group_id);

						ui::centered_text(is_group_view ? "No matching groups found..." : "No matching videos found...", inner_rect.GetSize(), table_start_cpos);
					}

					if (ImGui::BeginDragDropTargetCustom(inner_rect, ImGui::GetID("GroupDragDropPanel")))
					{
						auto payload = utils::drag_drop::get_payload<video_id_t>("Video", ImGuiDragDropFlags_AcceptBeforeDelivery);
						if (payload.data.has_value())
						{
							bool is_delivery = payload.imgui_payload->IsDelivery();

							video_group::video_info vinfo;
							vinfo.id = *payload.data;

							if (current_video_group != 0)
							{
								auto& vgroup = ctx_.current_project->video_groups.at(current_video_group);

								bool already_contains = vgroup.contains(vinfo.id);
								if (!is_delivery and already_contains)
								{
									ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
								}
								else if (is_delivery and !already_contains)
								{
									vgroup.insert(vinfo);
									ctx_.is_project_dirty = true;
									debug::log("Added video with id: {} to group with id: {}", vinfo.id, current_video_group);
								}
							}
							else if (is_delivery)
							{
								dragged_videos.push_back(vinfo.id);
								open_create_group_popup = true;
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::EndTable();
			}
		}
		else
		{
			ui::centered_text("Add groups to display them here...", ImGui::GetContentRegionMax());
		}

		static std::string group_name;
		if (open_create_group_popup)
		{
			ImGui::OpenPopup("Create New Group");
		}

		if (widgets::modal::create_group_popup("Create New Group", group_name))
		{
			auto id = utils::random::get_uuid();
			debug::log("Added video group with id: {}", id);
			auto [it, inserted] = ctx_.current_project->video_groups.insert({ id, video_group{} });
			if (inserted)
			{
				auto& group = it->second;
				group.display_name = group_name;
				for (auto& id : dragged_videos)
				{
					video_group::video_info vinfo;
					vinfo.id = id;
					group.insert(vinfo);
				}
				ctx_.is_project_dirty = true;
			}
			dragged_videos.clear();
			group_name.clear();
		}

		video_properties_popup_->open_and_render(open_properties_);
    }

	void video_group_browser::register_listeners()
	{
		ctx_.add_event_listener<video_group_remove_video_event>([](const video_group_remove_video_event& event)
		{
			ctx_.tasks.run_on_main([group_id = event.group_id(), video_id = event.video_id()]()
			{
				auto& vgroup = ctx_.current_project->video_groups.at(group_id);
				vgroup.erase(video_id);
				ctx_.is_project_dirty = true;
			});
		});

		ctx_.add_event_listener<video_open_properties_request_event>([this](const video_open_properties_request_event& event)
		{
			video_properties_popup_->set_video_id(event.video_id());
			video_properties_popup_->set_video_group_id(event.group_id());
			video_properties_popup_->set_offset(event.offset());
			open_properties_ = true;
		});

		ctx_.add_event_listener<video_change_offset_request_event>([this](const video_change_offset_request_event& event)
		{
			auto& group = ctx_.current_project->video_groups.at(event.group_id());
			auto it = group.find(event.video_id());
			if (it != group.end())
			{
				it->offset = event.offset();
				ctx_.is_project_dirty = true;

				ctx_.dispatch_event<video_change_offset_event>(get_event_source(), event.group_id(), event.video_id(), event.offset());
			}
		});
	}
}
