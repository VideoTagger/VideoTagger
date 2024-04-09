#include "pch.hpp"
#include "video_group_browser.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include "controls.hpp"

namespace vt::widgets
{
	void video_group_browser::render(bool& is_open)
	{
		if (!ctx_.current_project.has_value()) return;

		static auto draw_video_tile = [this](const video_pool::video_metadata& vmeta, ImVec2 tile_size, bool& open, bool& remove, SDL_Texture* image = nullptr)
		{
			std::string label = vmeta.path.stem().u8string();
			ImVec2 image_tile_size{ tile_size.x * 0.9f, tile_size.x * 0.9f };

			float scaled_width = vmeta.width * image_tile_size.y / vmeta.height;
			float scaled_height = image_tile_size.x * vmeta.height / vmeta.width;

			ImVec2 image_size = image_tile_size;
			if (scaled_width < image_tile_size.x)
			{
				image_size.x = scaled_width;
			}
			else if (scaled_height < image_tile_size.y)
			{
				image_size.y = scaled_height;
			}

			open = widgets::tile(label, tile_size, image_size, image,
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
			});
		};

		static auto draw_group_tile = [this](video_group& vgroup, video_group_id_t gid, ImVec2 tile_size, bool& open, bool& remove, bool& play)
		{
			open = widgets::tile(std::to_string(gid), tile_size, tile_size, nullptr,
			[&](const std::string& label)
			{
				if (ImGui::MenuItem("Play"))
				{
					play = true;
				}
				if (ImGui::MenuItem("Open"))
				{
					open = true;
				}
				if (ImGui::MenuItem("Remove"))
				{
					remove = true;
				}
			},
			[&](const std::string& label)
			{
				if (ImGui::BeginDragDropTarget())
				{
					auto payload = utils::drag_drop::get_payload<video_id_t>("Video");
					if (payload.has_value())
					{
						video_group::video_info vinfo;
						vinfo.id = *payload;
						if (!vgroup.contains(vinfo.id))
						{
							vgroup.insert(vinfo);
							ctx_.is_project_dirty = true;
							debug::log("Added video with id: {} to group with id: {}", vinfo.id, ctx_.current_video_group_id);
						}
					}
					ImGui::EndDragDropTarget();
				}
			});
		};

		auto& style = ImGui::GetStyle();

		if (ImGui::Begin("Video Group Browser", &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			if (ctx_.current_project->videos.size() > 0)
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

				static auto draw_group_tab = [&style](const std::string& group_name, video_group_id_t gid)
				{
					bool inactive = ctx_.current_video_group_id != gid;

					if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
					if (ImGui::TreeNodeEx(group_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf) and ImGui::IsItemClicked())
					{
						ctx_.current_video_group_id = gid;
					}
					if (inactive) ImGui::PopStyleColor();
				};

				if (ImGui::Button("Add New Group"))
				{
					auto id = utils::uuid::get();
					debug::log("Added empty vid group with id: {}", id);
					ctx_.current_project->video_groups.insert({ id, video_group{} });
					ctx_.is_project_dirty = true;
				}
				ImGui::SameLine();
				widgets::help_marker("This is temporary");
				ImGui::Separator();

				if (ImGui::BeginTable("##VideoBrowser", 2, ImGuiTableFlags_SizingStretchProp| ImGuiTableFlags_BordersInnerV))
				{
					ImGui::TableSetupColumn(nullptr, 0, 0.20f);
					ImGui::TableSetupColumn(nullptr, 0, 0.80f);

					ImGui::TableNextColumn();
					if (ImGui::BeginChild("##VideoBrowserGroupTabs"))
					{
						draw_group_tab("All Groups", 0);
						for (const auto& [gid, group] : ctx_.current_project->video_groups)
						{
							std::string group_name = std::to_string(gid);
							draw_group_tab(group_name, gid);
						}
						ImGui::EndChild();
					}

					ImGui::TableNextColumn();
					if (ImGui::BeginTable("##VideoBrowserBody", columns, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame, ImGui::GetContentRegionMax()))
					{
						ImGui::TableNextRow();
						if (ctx_.current_video_group_id == 0)
						{
							for (auto& [gid, group] : ctx_.current_project->video_groups)
							{
								bool open_group = false;
								bool remove_group = false;
								bool play_group = false;

								draw_group_tile(group, gid, tile_size, open_group, remove_group, play_group);
								if (remove_group)
								{
									ctx_.is_project_dirty = true;
									ctx_.current_project->video_groups.erase(gid);
									break;
								}

								if (open_group)
								{
									debug::log("Opening group {}", gid);
									ctx_.current_video_group_id = gid;
								}

								if (play_group)
								{
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
								}
							}
						}
						else
						{
							auto& pool = ctx_.current_project->videos;
							auto& vgroup = ctx_.current_project->video_groups.at(ctx_.current_video_group_id);
							for (auto& vinfo : ctx_.videos_manager)
							{
								auto metadata = pool.get(vinfo.id);
								if (metadata == nullptr) continue;

								bool open_video{};
								bool remove_video{};
								draw_video_tile(*metadata, tile_size, open_video, remove_video, metadata->thumbnail);
								if (remove_video)
								{
									auto& vgroup = ctx_.current_project->video_groups.at(ctx_.current_video_group_id);
									vgroup.erase(vinfo.id);
									ctx_.is_project_dirty = true;
								}

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

								if (ImGui::IsWindowFocused() and ImGui::IsKeyPressed(ImGuiKey_Escape))
								{
									ctx_.current_video_group_id = 0;
								}
							}
						}
						ImGui::EndTable();
					}
					if (ctx_.current_video_group_id != 0)
					{
						ImRect inner_rect = ImGui::GetCurrentWindow()->InnerRect;
						if (ImGui::BeginDragDropTargetCustom(inner_rect, ImGui::GetID("GroupDragDropPanel")))
						{
							auto payload = utils::drag_drop::get_payload<video_id_t>("Video");
							if (payload.has_value())
							{
								video_group::video_info vinfo;
								vinfo.id = *payload;
								auto& vgroup = ctx_.current_project->video_groups.at(ctx_.current_video_group_id);
								if (!vgroup.contains(vinfo.id))
								{
									vgroup.insert(vinfo);
									ctx_.is_project_dirty = true;
									debug::log("Added video with id: {} to group with id: {}", vinfo.id, ctx_.current_video_group_id);
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
		}
		ImGui::End();
	}
}
