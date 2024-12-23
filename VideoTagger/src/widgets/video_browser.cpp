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

		static auto draw_video_tile = [this](video_id_t id, video_resource& vid_resource, ImVec2 tile_size, bool& open, GLuint image = 0)
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

			open = widgets::tile(fmt::format("video{}", id).c_str(), label, tile_size, image_size, image,
			[&](const std::string& label)
			{
				std::vector<video_resource_context_menu_item> context_items;
				vid_resource.context_menu_items(context_items);
				for (auto& item : context_items)
				{
					if (item.disabled) ImGui::BeginDisabled(item.disabled);

					if (ImGui::MenuItem(item.name.c_str()))
					{
						item.function();
					}

					if (item.disabled) ImGui::EndDisabled();

					if (!item.tooltip.empty())
					{
						widgets::tooltip(item.tooltip.c_str());
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
			[&vid_resource](ImDrawList& draw_list, ImRect item_rect, ImRect image_rect)
			{
				vid_resource.icon_custom_draw(draw_list, item_rect, image_rect);
			},
			uv0, uv1);
		};

		auto& style = ImGui::GetStyle();

		if (ImGui::Begin(window_name().c_str(), &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			bool any_item_hovered = false;
			bool window_hovered = false;
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

									ImGui::TableNextColumn();
									draw_video_tile(vid_resource->id(), *vid_resource, tile_size, open_video, vid_resource->thumbnail() ? vid_resource->thumbnail()->id() : 0);
									
									any_item_hovered = any_item_hovered or ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
								}

								//ImGui::EndTable();
							//}
						//}
					}

					window_hovered = ImGui::IsWindowHovered();

					ImGui::EndTable();
				}
			}
			else
			{
				window_hovered = ImGui::IsWindowHovered();
				widgets::centered_text("Import videos to display them here...", ImGui::GetContentRegionMax());
			}

			if (!any_item_hovered and ImGui::IsMouseReleased(ImGuiMouseButton_Right) and window_hovered)
			{
				ImGui::OpenPopup("##BrowserContextMenu", ImGuiPopupFlags_NoOpenOverExistingPopup);
			}

			if (ImGui::BeginPopup("##BrowserContextMenu"))
			{
				if (!ctx_.current_project->videos.empty())
				{
					std::string refresh_item_name = fmt::format("{} {}", icons::refresh, "Refresh Videos");
					if (ImGui::MenuItem(refresh_item_name.c_str()))
					{
						for (auto& [id, vid_resource] : ctx_.current_project->videos)
						{
							ctx_.current_project->schedule_video_refresh(id);
						}
					}
				}
				
				for (auto& [importer_id, importer] : ctx_.video_importers)
				{
					if (!importer->available())
					{
						continue;
					}

					std::string item_name = fmt::format("{} Import From {}", importer->importer_display_icon(), importer->importer_display_name());
					if (ImGui::MenuItem(item_name.c_str()))
					{
						ctx_.current_project->prepare_video_import(importer_id);
					}
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	std::string video_browser::window_name()
	{
		return fmt::format("{} Video Browser", icons::database);
	}
}
