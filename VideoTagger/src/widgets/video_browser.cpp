#include "pch.hpp"
#include "video_browser.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include "controls.hpp"

namespace vt::widgets
{
	void video_browser::render(bool& is_open)
	{
		if (!ctx_.current_project.has_value()) return;

		static auto draw_video_tile = [this](video_id_t id, const video_pool::video_metadata& vmeta, ImVec2 tile_size, bool& open, bool& remove, SDL_Texture* image = nullptr)
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
				if (ImGui::MenuItem("Open"))
				{
					open = true;
				}
				if (ImGui::MenuItem("Remove"))
				{
					remove = true;
				}
			},
			[=](const std::string& label)
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) //ImGuiDragDropFlags_SourceNoPreviewTooltip
				{
					utils::drag_drop::set_payload("Video", id);
					ImGui::Image(static_cast<ImTextureID>(image), image_size / 2);
					ImGui::EndDragDropSource();
				}
			});
		};

		auto& style = ImGui::GetStyle();

		if (ImGui::Begin("Video Browser", &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{			
			if (ctx_.current_project->videos.size() > 0)
			{
				ImVec2 img_tile_size{ ctx_.app_settings.thumbnail_size, ctx_.app_settings.thumbnail_size };
				ImVec2 tile_size = img_tile_size + style.ItemSpacing + style.CellPadding / 2;

				auto avail = ImGui::GetContentRegionAvail() - ImVec2{ 0, ImGui::GetTextLineHeightWithSpacing() };
				int columns = static_cast<int>(avail.x / tile_size.x);
				if (columns < 1)
				{
					columns = 1;
				}

				if (ImGui::BeginTable("##VideoBrowserBody", columns, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame, ImGui::GetContentRegionMax()))
				{
					ImGui::TableNextRow();
					for (auto& [id, metadata] : ctx_.current_project->videos)
					{
						bool open_video{};
						bool remove_video{};

						draw_video_tile(id, metadata, tile_size, open_video, remove_video, metadata.thumbnail);
						if (remove_video)
						{
							debug::log("Removing video with id: {}", id);
							ctx_.current_project->videos.erase(id);
							break;
						}

						/*
						if (open_video and !metadata.is_widget_open)
						{
							debug::log("Opening video {}", metadata.path.u8string());
							if (on_open_video != nullptr)
							{
								std::invoke(on_open_video, id);
							}
						}
						*/
					}
					ImGui::EndTable();
				}
			}
			else
			{
				widgets::centered_text("Import videos to display them here...", ImGui::GetContentRegionMax());
			}
		}
		ImGui::End();
	}
}
