#include "pch.hpp"
#include "video_group_queue.hpp"

#include <core/app_context.hpp>
#include <core/debug.hpp>
#include <utils/drag_drop.hpp>
#include "controls.hpp"
#include <utils/thumbnail.hpp>
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>

#include <events/player/video_group_change_request_event.hpp>

namespace vt::widgets
{
	video_group_queue::video_group_queue() : ui::window{ "Group Queue", "video-group-queue", "Group Queue", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize }
	{
		set_icon(icons::queue);
	}

	void video_group_queue::set_current_group_id(video_group_id_t id)
	{
		current_group_id_ = id;
	}

	video_group_id_t video_group_queue::current_group_id() const
	{
		return current_group_id_;
	}

	void video_group_queue::pre_render()
	{
		set_current_group_id(ctx_.session.current_video_group_id());
	}

	void video_group_queue::on_render()
    {
		ImVec2 tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
		ImVec2 image_tile_size{ tile_size.x * 0.9f, tile_size.x * 0.9f };
		auto sel_tile_size = calc_selectable_tile_size(tile_size);

		auto& playlist = ctx_.current_project->video_group_playlist;
		bool is_shuffled = playlist.is_shuffled();
		const auto& style = ImGui::GetStyle();
		const auto& theme = ctx_.current_theme;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
		bool table_open = ImGui::BeginTable("##GroupQueuePanels", 2, ImGuiTableFlags_BordersInnerV);
		ImGui::PopStyleVar();

		if (table_open)
		{
			float icon_column_size = ImGui::CalcTextSize(icons::play_queue).x + 2 * style.FramePadding.x + 2 * style.CellPadding.x;

			ImGui::TableSetupColumn(nullptr);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed, icon_column_size);

			ImGui::TableNextColumn();
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, theme.get_float4(theme_color::background_secondary));
				bool child_open = ImGui::BeginChild("##GroupQueueList", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::PopStyleColor();

				if (child_open)
				{
					auto& groups = ctx_.current_project->video_groups;

					const auto& style = ImGui::GetStyle();

					ImVec2 spacer_size{ 1.25f * style.ItemSpacing.x, sel_tile_size.y };

					static auto draw_spacer = [&playlist](ImVec2 spacer_size, video_group_playlist::iterator& it)
					{
						ImGui::Dummy(spacer_size);
						if (ImGui::BeginDragDropTarget())
						{
							auto payload = utils::drag_drop::get_payload<video_group_id_t>("Group", ImGuiDragDropFlags_AcceptBeforeDelivery);

							if (payload.data.has_value())
							{
								auto& current_project = *ctx_.current_project;

								bool is_delivery = payload.imgui_payload->IsDelivery();

								bool already_contains = std::find(playlist.begin(), playlist.end(), payload.data.value()) != playlist.end();
								bool contains_unplayable = false;
								for (auto& video : current_project.videos.get_group(current_project.video_groups.at(payload.data.value())))
								{
									if (video->playable())
									{
										continue;
									}

									contains_unplayable = true;
									break;
								}

								if (!is_delivery and (already_contains or contains_unplayable))
								{
									ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
								}
								else if (is_delivery and !(already_contains or contains_unplayable))
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

					std::optional<video_group_playlist::iterator> clicked_group;
					for (; it != playlist.end();)
					{
						bool remove_group{};
						const auto& group_id = *it;

						auto& group = groups.at(group_id);
						std::string label = group.display_name;

						ImGui::SameLine();
						draw_spacer(spacer_size, it);

						ImGui::SameLine();
						bool is_selected = current_group_id_ != 0 and current_group_id_ == group_id;
						auto tint_color = theme.get_float4(theme_color::icon_thumbnail);

						bool tile_active = tile(fmt::format("group{}", group_id).c_str(), label, tile_size, image_tile_size, image,
							[&remove_group](const std::string& label)
						{
							if (ImGui::MenuItem("Remove"))
							{
								remove_group = true;
							}
						},
							[&remove_group, gid = group_id](const std::string& label)
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
						}, nullptr, glyph.uv0, glyph.uv1, is_selected, tint_color);

						if (tile_active)
						{
							clicked_group = it;
						}

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

					if (clicked_group.has_value())
					{
						ctx_.dispatch_event<video_group_change_request_event>("group queue", ctx_.get_window<widgets::video_player>(), **clicked_group);
						clicked_group.reset();
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
								auto& current_project = *ctx_.current_project;

								bool is_delivery = payload.imgui_payload->IsDelivery();

								bool already_contains = std::find(playlist.begin(), playlist.end(), payload.data.value()) != playlist.end();
								bool contains_unplayable = false;
								for (auto& video : current_project.videos.get_group(current_project.video_groups.at(payload.data.value())))
								{
									if (video->playable())
									{
										continue;
									}

									contains_unplayable = true;
									break;
								}

								if (!is_delivery and (already_contains or contains_unplayable))
								{
									ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
								}
								else if (is_delivery and !(already_contains or contains_unplayable))
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
				bool can_play = !is_empty and ctx_.session.current_video_group_id() == invalid_video_group_id;
				auto cpos = ImGui::GetCursorPosX() + style.CellPadding.x;
				ImGui::SetCursorPosX(cpos);
				if (!can_play) ImGui::BeginDisabled();
				if (ui::icon_button(icons::play_queue))
				{
					auto it = playlist.set_current(playlist.begin());
					ctx_.dispatch_event<video_group_change_request_event>("group queue", ctx_.get_window<widgets::video_player>(), *it);

					auto& pool = ctx_.current_project->videos;
				}
				if (!can_play) ImGui::EndDisabled();
				ui::tooltip("Play");

				ImGui::SetCursorPosX(cpos);
				if (is_empty) ImGui::BeginDisabled();
				if (ui::icon_button(icons::delete_))
				{
					playlist.clear();
					ctx_.is_project_dirty = true;
				}
				if (is_empty) ImGui::EndDisabled();
				ui::tooltip("Clear queue");

				ImGui::SetCursorPosX(cpos);
				if (ui::icon_toggle_button(icons::shuffle, is_shuffled))
				{
					playlist.set_shuffle(!is_shuffled);
				}
				ui::tooltip(is_shuffled ? "Disable shuffling" : "Enable shuffling");
			}
			ImGui::EndTable();
		}
    }

	void video_group_queue::pre_style()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
	}

	void video_group_queue::post_style()
	{
		ImGui::PopStyleVar();
	}
}
