#include "pch.hpp"
#include "video_group_queue.hpp"

#include <core/app_context.hpp>
#include <core/debug.hpp>
#include <utils/drag_drop.hpp>
#include "controls.hpp"
#include <utils/thumbnail.hpp>
#include "icons.hpp"

namespace vt::widgets
{
	void video_group_queue::render(bool& is_open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		bool win_open = ImGui::Begin("Group Queue", &is_open, ImGuiWindowFlags_NoCollapse);
		ImGui::PopStyleVar();

		if (win_open)
		{
			auto& playlist = ctx_.current_project->video_group_playlist;
			bool is_shuffled = playlist.is_shuffled();
			const auto& style = ImGui::GetStyle();
			
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
			bool table_open = ImGui::BeginTable("##GroupQueuePanels", 2, ImGuiTableFlags_BordersInnerV);
			ImGui::PopStyleVar();

			if (table_open)
			{
				float icon_column_size = ImGui::CalcTextSize(icons::play).x + 2 * style.FramePadding.x + 2 * style.CellPadding.x;

				ImGui::TableSetupColumn(nullptr);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, icon_column_size);

				ImGui::TableNextColumn();
				{
					ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_MenuBarBg]);
					bool child_open = ImGui::BeginChild("##GroupQueueList", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_HorizontalScrollbar);
					ImGui::PopStyleColor();

					if (child_open)
					{
						auto& groups = ctx_.current_project->video_groups;

						const auto& style = ImGui::GetStyle();
						ImVec2 tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
						ImVec2 image_tile_size{ tile_size.x * 0.9f, tile_size.x * 0.9f };
						auto sel_tile_size = calc_selectable_tile_size(tile_size);

						ImVec2 spacer_size{ 1.25f * style.ItemSpacing.x, sel_tile_size.y };

						static auto draw_spacer = [&playlist](ImVec2 spacer_size, video_group_playlist::iterator& it)
						{
							ImGui::Dummy(spacer_size);
							if (ImGui::BeginDragDropTarget())
							{
								auto payload = utils::drag_drop::get_payload<video_group_id_t>("Group", ImGuiDragDropFlags_AcceptBeforeDelivery);

								if (payload.data.has_value())
								{
									bool is_delivery = payload.imgui_payload->IsDelivery();

									bool already_contains = std::find(playlist.begin(), playlist.end(), payload.data.value()) != playlist.end();
									if (!is_delivery and already_contains)
									{
										ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
									}
									else if (is_delivery and !already_contains)
									{
										it = playlist.insert(it, payload.data.value());
									}
								}

								ImGui::EndDragDropTarget();
							}
						};

						auto image = utils::thumbnail::font_texture();
						auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_group_icon);

						size_t playlist_size = playlist.size();
						auto it = playlist.begin();
						auto full_avail_area = ImGui::GetContentRegionAvail();

						for (; it != playlist.end();)
						{
							bool remove_group{};
							const auto& group_id = *it;

							auto& group = groups.at(group_id);
							std::string label = group.display_name;

							ImGui::SameLine();
							draw_spacer(spacer_size, it);

							ImGui::SameLine();
							bool is_selected = current_group_id != 0 and current_group_id == group_id;

							tile(label, tile_size, image_tile_size, image, [&remove_group](const std::string& label)
							{
								if (ImGui::MenuItem("Remove"))
								{
									remove_group = true;
								}
							},
							[&remove_group, gid = group_id, label](const std::string& label)
							{
								//TODO: This will display "..." when it gets removed, try to fix that
								if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
								{
									if (ImGui::IsMouseDown(0))
									{
										remove_group = true;
									}

									utils::drag_drop::set_payload("Group", gid);
									std::string str = fmt::format("{} {}", icons::video_group, label);
									ImGui::TextUnformatted(str.c_str());
									ImGui::EndDragDropSource();
								}
							}, glyph.uv0, glyph.uv1, is_selected);

							ImGui::SameLine();

							if (remove_group)
							{
								it = playlist.erase(it);
							}
							else
							{
								++it;
							}
						}

						//draw_spacer(spacer_size, it);
						{
							auto win_rect = ImGui::GetCurrentWindow()->InnerRect;
							auto avail_area = ImGui::GetContentRegionAvail();
							auto offset = ImGui::GetContentRegionMax() - avail_area;
							auto rect_size = win_rect.GetTL() + ImVec2{ full_avail_area.x, spacer_size.y };
							ImRect inner_rect = { win_rect.GetTL() + offset, rect_size };
							if (ImGui::BeginDragDropTargetCustom(inner_rect, ImGui::GetID("GroupQueueDragDropPanel")))
							{
								auto payload = utils::drag_drop::get_payload<video_group_id_t>("Group", ImGuiDragDropFlags_AcceptBeforeDelivery);

								if (payload.data.has_value())
								{
									bool is_delivery = payload.imgui_payload->IsDelivery();

									bool already_contains = std::find(playlist.begin(), playlist.end(), payload.data.value()) != playlist.end();
									if (!is_delivery and already_contains)
									{
										ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
									}
									else if (is_delivery and !already_contains)
									{
										it = playlist.insert(it, payload.data.value());
									}
								}

								ImGui::EndDragDropTarget();
							}
						}
					}
					ImGui::EndChild();
				}

				ImGui::TableNextColumn();
				{
					bool is_empty = playlist.empty();
					bool can_play = !is_empty and ctx_.current_video_group_id() == invalid_video_group_id;
					auto cpos = ImGui::GetCursorPosX() + style.CellPadding.x;
					ImGui::SetCursorPosX(cpos);
					if (!can_play) ImGui::BeginDisabled();
					if (icon_button(icons::play))
					{
						auto it = playlist.set_current(playlist.begin());
						ctx_.set_current_video_group_id(*it);

						auto& pool = ctx_.current_project->videos;
					}
					if (!can_play) ImGui::EndDisabled();
					tooltip("Play");

					ImGui::SetCursorPosX(cpos);
					if (is_empty) ImGui::BeginDisabled();
					if (icon_button(icons::delete_))
					{
						playlist.clear();
					}
					if (is_empty) ImGui::EndDisabled();
					tooltip("Clear queue");

					ImGui::SetCursorPosX(cpos);
					if (icon_toggle_button(icons::shuffle, is_shuffled))
					{
						playlist.set_shuffle(!is_shuffled);
					}
					tooltip(is_shuffled ? "Disable shuffling" : "Enable shuffling");
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}
}
