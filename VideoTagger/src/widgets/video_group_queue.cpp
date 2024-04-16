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
		if (ImGui::Begin("Group Queue", &is_open))
		{
			ImGui::Button("Play queue");
			ImGui::Separator();

			if (ImGui::BeginChild("##GroupQueueList", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_HorizontalScrollbar))
			{
				auto& groups = ctx_.current_project->video_groups;
				auto& playlist = ctx_.current_project->video_group_playlist;

				const auto& style = ImGui::GetStyle();
				ImVec2 tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
				ImVec2 image_tile_size{ tile_size.x * 0.9f, tile_size.x * 0.9f };

				ImVec2 spacer_size{ style.ItemSpacing.x, tile_size.y };

				static auto draw_spacer = [&playlist](ImVec2 spacer_size, video_group_playlist::iterator& it)
				{
					ImGui::Dummy(spacer_size);
					if (ImGui::BeginDragDropTarget())
					{
						auto payload = utils::drag_drop::get_payload<video_group_id_t>("Group");
						if (payload.has_value())
						{
							if (!playlist.contains(payload.value()))
							{
								it = playlist.insert(it, payload.value());
							}
						}
						ImGui::EndDragDropTarget();
					}
				};

				SDL_Texture* image = utils::thumbnail::font_texture();
				auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_group_icon);

				size_t playlist_size = playlist.size();
				size_t i{};
				auto it = playlist.begin();

				//TODO: Inserting here puts value at the 2nd element, not 1st

				for (; it != playlist.end();)
				{
					bool remove_group{};
					const auto& pinfo = *it;

					auto& group = groups.at(pinfo.group_id);
					std::string label = group.display_name;
					
					draw_spacer(spacer_size, it);

					ImGui::SameLine();
					tile(label, tile_size, image_tile_size, image, [&remove_group](const std::string& label)
					{
						if (ImGui::MenuItem("Remove"))
						{
							remove_group = true;
						}
					}, nullptr, glyph.uv0, glyph.uv1);
					
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

				draw_spacer(spacer_size, it);
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}
