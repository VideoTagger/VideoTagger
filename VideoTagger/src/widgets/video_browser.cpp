#define IMGUI_DEFINE_MATH_OPERATORS
#include "video_browser.hpp"
#include <cstdint>
#include <string>

#include <SDL.h>
#include <imgui.h>
#include <core/debug.hpp>
#include <core/app_context.hpp>
#include "controls.hpp"

namespace vt::widgets
{
	void video_browser::render(bool& is_open)
	{
		if (!ctx_.current_project.has_value()) return;

		static auto draw_video_tile = [this](const video_pool::video_metadata& vinfo, ImVec2 image_tile_size, ImVec2 tile_size, bool& open, bool& remove, SDL_Texture* texture = nullptr)
		{
			int video_width = vinfo.width;
			int video_height = vinfo.height;

			float scaled_width = video_width * image_tile_size.y / video_height;
			float scaled_height = image_tile_size.x * video_height / video_width;

			ImVec2 image_size = image_tile_size;
			if (scaled_width < image_tile_size.x)
			{
				image_size.x = scaled_width;
			}
			else if (scaled_height < image_tile_size.y)
			{
				image_size.y = scaled_height;
			}

			auto& style = ImGui::GetStyle();
			std::string name = vinfo.path.stem().u8string();
			ImGui::TableNextColumn();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImTextureID imgui_tex = static_cast<ImTextureID>(texture);
			const char* id = name.c_str();
			ImGui::PushID(id);
			bool selected = false;
			auto text_size = ImVec2{ 0, ImGui::CalcTextSize(id, nullptr, false, tile_size.x).y };
			auto selectable_size = tile_size + style.FramePadding + text_size;
			ImVec2 cpos = ImGui::GetCursorPos() + (selectable_size - image_size - text_size) / 2;
			if (ImGui::Selectable("##VideoTileButton", &selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick, selectable_size))
			{
				
			}
			if (ImGui::IsItemActivated() and ImGui::IsMouseDoubleClicked(0))
			{
				open = true;
			}
			if (ImGui::BeginPopupContextItem("##VideoTileCtxMenu"))
			{
				if (!vinfo.is_widget_open and ImGui::MenuItem("Open"))
				{
					open = true;
				}
				if (ImGui::MenuItem("Remove"))
				{
					remove = true;
				}
				ImGui::EndPopup();
			}

			ImGui::SetCursorPos(std::exchange(cpos, ImGui::GetCursorPos()));
			ImGui::BeginGroup();
			ImGui::Image(imgui_tex, image_size);
			ImGui::Dummy({ 0, (image_tile_size.y - image_size.y) / 2.f });
			ImGui::TextWrapped(id);
			ImGui::EndGroup();
			ImGui::SetCursorPos(cpos);

			ImGui::PopID();
			ImGui::PopStyleVar();
		};

		auto& style = ImGui::GetStyle();

		if (ImGui::Begin("Video Browser", &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			float thumbnail_size = 84.0f;
			float text_height = ImGui::GetTextLineHeight();

			ImVec2 img_size{ thumbnail_size, thumbnail_size };
			img_size.x -= text_height;
			img_size.y -= text_height;

			ImVec2 tile_size = img_size + style.ItemSpacing + style.CellPadding / 2;
			auto avail = ImGui::GetContentRegionAvail() - ImVec2{ 0, ImGui::GetTextLineHeightWithSpacing() };
			int columns = static_cast<int>(avail.x / tile_size.x);
			if (columns < 1)
			{
				columns = 1;
			}

			if (ctx_.current_project->videos.size() > 0)
			{
				if (ImGui::BeginTable("##VideoDrawer", columns, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame))
				{
					ImGui::TableNextRow();
					for (const auto& [id, vinfo] : ctx_.current_project->videos)
					{
						bool open = false;
						bool remove = false;
						draw_video_tile(vinfo, img_size, tile_size, open, remove, vinfo.thumbnail);
						if (remove)
						{
							ctx_.current_project->videos.erase(id);
							break;
						}

						if (open and !vinfo.is_widget_open)
						{
							debug::log("Opening video " + vinfo.path.string());
							if (on_open_video != nullptr)
							{
								std::invoke(on_open_video, id);
							}
						}
					}
					ImGui::EndTable();
				}
			}
			else
			{
				auto avail_area = ImGui::GetContentRegionMax();
				constexpr const char* text = "Import videos to display them here...";
				auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
				ImGui::SetCursorPos(avail_area / 2 - half_text_size);
				ImGui::BeginDisabled();
				ImGui::TextWrapped(text);
				ImGui::EndDisabled();
			}
		}
		ImGui::End();
	}
}
