#include "pch.hpp"
#include "video_browser.hpp"

#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <utils/thumbnail.hpp>
#include <widgets/controls.hpp>
#include "controls.hpp"
#include "icons.hpp"

namespace vt::widgets
{
	void video_browser::render(bool& is_open)
	{
		if (!ctx_.current_project.has_value()) return;

		static auto draw_video_tile = [this](video_id_t id, video_resource& vid_resource, ImVec2 tile_size, bool& open, bool& remove, GLuint image = 0)
		{
			const auto& metadata = vid_resource.metadata();
			std::string label = metadata.title.value_or("");
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
				float scaled_width = *metadata.width * image_tile_size.y / *metadata.height;
				float scaled_height = image_tile_size.x * *metadata.height / *metadata.width;

				if (scaled_width < image_tile_size.x)
				{
					image_size.x = scaled_width;
				}
				else if (scaled_height < image_tile_size.y)
				{
					image_size.y = scaled_height;
				}
			}

			open = widgets::tile(label, tile_size, image_size, image,
			[&](const std::string& label)
			{
				if (ImGui::MenuItem("Remove"))
				{
					remove = true;
				}
				std::vector<video_resource_context_menu_item> context_items;
				vid_resource.context_menu_items(context_items);
				for (auto& item : context_items)
				{
					if (ImGui::MenuItem(item.name.c_str()))
					{
						item.function();
					}
				}
			},
			[=](const std::string& label)
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) //ImGuiDragDropFlags_SourceNoPreviewTooltip
				{
					utils::drag_drop::set_payload("Video", id);
					std::string text = fmt::format("{} {}", icons::video, label);
					ImGui::TextUnformatted(text.c_str());
					ImGui::EndDragDropSource();
				}
			},
			//TODO: maybe make a function in video resource that returns custom draw
			[&](ImDrawList& draw_list, ImRect item_rect, ImRect image_rect)
			{
				downloadable_video_resource* vid = dynamic_cast<downloadable_video_resource*>(&vid_resource);
				if (vid == nullptr)
				{
					return;
				}

				auto download_progress = vid->download_progress();
				if (download_progress.has_value())
				{
					float progress_bar_width = image_rect.GetWidth() * *download_progress;
					ImVec2 progress_bar_min = image_rect.Min;
					ImVec2 progress_bar_max = { image_rect.Min.x + progress_bar_width, image_rect.Max.y };

					draw_list.AddRectFilled(progress_bar_min, progress_bar_max, ImGui::ColorConvertFloat4ToU32({ 0.f, 1.f, 0.f, 0.75f }));
				}
				else
				{
					if (!vid_resource.available())
					{
						//TODO: change this
						draw_list.AddText(ImGui::GetFont(), 50.f, image_rect.Min, ImGui::ColorConvertFloat4ToU32({ 1.f, 0.f, 0.f, 1.f }), icons::download);
						//draw_list.AddRectFilled(item_rect.Min, item_rect.Max, ImGui::ColorConvertFloat4ToU32({ 1.f, 0.f, 0.f, 1.f }));
					}
				}
			}, uv0, uv1);
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

				std::unordered_map<std::string, std::vector<video_resource*>> grouped_videos;
				for (auto& [id, vid_resource] : ctx_.current_project->videos)
				{
					grouped_videos[vid_resource->importer_id()].push_back(vid_resource.get());
				}

				auto table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame;
				if (ImGui::BeginTable("##VideoBrowserBody", columns, table_flags, ImGui::GetContentRegionMax()))
				{
					ImGui::TableNextRow();

					//auto node_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth;
					for (auto& [importer_id, vid_resources] : grouped_videos)
					{
						//if (collapsing_header(ctx_.video_importers.at(importer_id)->importer_display_name().c_str()))
						//{
							//std::string table_id = fmt::format("##VideoBrowser{}", importer_id);
							//if (ImGui::BeginTable(table_id.c_str(), columns, ImGuiTableFlags_SizingFixedFit))
							//{
								for (auto& vid_resource : vid_resources)
								{
									bool open_video{};
									bool remove_video{};

									ImGui::TableNextColumn();
									draw_video_tile(vid_resource->id(), *vid_resource, tile_size, open_video, remove_video, vid_resource->thumbnail() ? vid_resource->thumbnail()->id() : 0);
									if (remove_video)
									{
										debug::log("Removing video with id: {}", vid_resource->id());
										ctx_.current_project->remove_video(vid_resource->id());
										break;
									}
								}

								//ImGui::EndTable();
							//}
						//}
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
