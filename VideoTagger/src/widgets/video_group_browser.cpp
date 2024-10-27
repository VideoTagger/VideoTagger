#include "pch.hpp"
#include "video_group_browser.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <utils/thumbnail.hpp>
#include <utils/string.hpp>
#include "modal/create_group_popup.hpp"
#include "modal/video_properties_popup.hpp"
#include "icons.hpp"
#include "controls.hpp"

namespace vt::widgets
{
	void video_group_browser::render(bool& is_open)
	{
		if (!ctx_.current_project.has_value()) return;

		static auto draw_video_tile = [this](const video_pool::video_metadata& vmeta, ImVec2 tile_size, bool& open, bool& remove, bool& properties, GLuint image = 0)
		{
			std::string label = vmeta.path.filename().u8string();
			ImVec2 image_tile_size{ tile_size.x * 0.9f, tile_size.x * 0.9f };

			ImVec2 image_size = image_tile_size;
			ImVec2 uv0{ 0, 0 };
			ImVec2 uv1{ 1, 1 };
			if (image == 0)
			{
				image = utils::thumbnail::font_texture();
				auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_icon);
				uv0 = glyph.uv0;
				uv1 = glyph.uv1;
			}
			else
			{
				float scaled_width = vmeta.width * image_tile_size.y / vmeta.height;
				float scaled_height = image_tile_size.x * vmeta.height / vmeta.width;

				if (scaled_width < image_tile_size.x)
				{
					image_size.x = scaled_width;
				}
				else if (scaled_height < image_tile_size.y)
				{
					image_size.y = scaled_height;
				}
			}
			open |= widgets::tile(label, tile_size, image_size, image,
			[&](const std::string& label)
			{
				//TODO: Temporarily disabled, enable this later
				/*
				if (ImGui::MenuItem("Open"))
				{
					open = true;
				}
				*/
				if (ImGui::MenuItem("Remove"))
				{
					remove = true;
				}
				if (ImGui::MenuItem("Properties"))
				{
					properties = true;
				}
			}, nullptr, uv0, uv1);
		};

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
			ImGui::PushID((void*)gid);

			auto image = utils::thumbnail::font_texture();
			auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_group_icon);

			open |= widgets::tile(vgroup.display_name, tile_size, tile_size, image,
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
			glyph.uv0, glyph.uv1);
			ImGui::PopID();
		};

		auto& style = ImGui::GetStyle();
		bool open_create_group_popup{};
		//TODO: This should be inside the payload, not here
		static std::vector<video_id_t> dragged_videos;

		if (ImGui::Begin("Video Group Browser", &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			//TODO: remove this if
			if (true or ctx_.current_project->videos.size() > 0)
			{
				ImVec2 img_tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
				ImVec2 tile_size = img_tile_size + style.ItemSpacing + style.CellPadding / 2;
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
					if (icon_button(icons::add))
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
					if (icon_button(icons::back))
					{
						current_video_group = 0;
					}
					if (!can_back_out) ImGui::EndDisabled();
					ImGui::SameLine();
					search_bar("##VideoGroupBrowserSearch", "Search...", filter);
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
								ctx_.current_project->remove_video_group(gid);
								if (current_video_group == gid)
								{
									current_video_group = 0;
								}
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
										if (on_open_video != nullptr)
										{
											std::invoke(on_open_video, vinfo.id);
										}
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
								draw_group_tile(group, gid, tile_size, open_group, remove_group, enqueue_group, can_enqueue);
								if (remove_group)
								{
									ctx_.current_project->remove_video_group(gid);
									if (current_video_group = gid)
									{
										current_video_group = 0;
									}
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
											if (on_open_video != nullptr)
											{
												std::invoke(on_open_video, vinfo.id);
											}
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
								auto metadata = pool.get(vinfo.id);
								if (metadata == nullptr) continue;

								bool open_video{};
								bool remove_video{};
								bool open_video_properties{};

								//filtering
								{
									bool passes_filter = true;
									for (const auto& token : tokens)
									{
										auto ttoken = utils::string::trim_whitespace(token);
										std::string name = utils::string::to_lowercase(metadata->path.filename().u8string());
										passes_filter &= name.find(ttoken) != std::string::npos;
									}

									if (!passes_filter) continue;
									++filter_passes;
								}

								ImGui::TableNextColumn();
								draw_video_tile(*metadata, tile_size, open_video, remove_video, open_video_properties, metadata->thumbnail ? metadata->thumbnail->id() : 0);
								if (remove_video)
								{
									auto& vgroup = ctx_.current_project->video_groups.at(current_video_group);
									vgroup.erase(vinfo.id);
									ctx_.is_project_dirty = true;
									break;
								}

								static std::chrono::nanoseconds offset;
								ImGui::PushID((void*)vinfo.id);
								if (open_video_properties)
								{
									offset = vinfo.offset;
									ImGui::OpenPopup("Video Properties");
								}

								if (video_properties_popup("Video Properties", offset))
								{
									vinfo.offset = offset;
								}
								ImGui::PopID();

								/*
								if (open_video and !metadata->is_widget_open)
								{
									debug::log("Opening video {}", metadata->path.u8string());
									if (on_open_video != nullptr)
									{
										std::invoke(on_open_video, vinfo.id);
									}
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

							centered_text(is_group_view ? "No matching groups found..." : "No matching videos found...", inner_rect.GetSize(), table_start_cpos);
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
				widgets::centered_text("Add groups to display them here...", ImGui::GetContentRegionMax());
			}
			
			static std::string group_name;
			if (open_create_group_popup)
			{
				ImGui::OpenPopup("Create New Group");
			}

			if (widgets::modal::create_group_popup("Create New Group", group_name))
			{
				auto id = utils::uuid::get();
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
		}
		ImGui::End();
	}
}
