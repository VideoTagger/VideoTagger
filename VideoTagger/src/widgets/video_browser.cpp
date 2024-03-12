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

		static auto draw_video_tile = [](const std::filesystem::path& path, ImVec2 img_size, ImVec2 tile_size, SDL_Texture* texture = nullptr)
		{
			auto& style = ImGui::GetStyle();
			std::string name = path.stem().u8string();
			ImGui::TableNextColumn();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImTextureID imgui_tex = static_cast<ImTextureID>(texture);
			const char* id = name.c_str();
			ImGui::PushID(id);
			bool selected = false;
			auto text_size = ImVec2{ 0, ImGui::CalcTextSize(id, nullptr, false, tile_size.x).y };
			auto selectable_size = tile_size + style.FramePadding + text_size;
			ImVec2 cpos = ImGui::GetCursorPos() + (selectable_size - img_size - text_size) / 2;
			if (ImGui::Selectable("##VideoTileButton", &selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick, selectable_size))
			{
				
			}
			if (ImGui::IsItemActivated() and ImGui::IsMouseDoubleClicked(0))
			{
				debug::log("Double clicked " + name + " in video browser");
			}
			std::string path_str = std::filesystem::relative(path, ctx_.current_project->path.parent_path()).u8string();
			ImGui::SetItemTooltip(path_str.c_str());
			ImGui::SetCursorPos(std::exchange(cpos, ImGui::GetCursorPos()));
			ImGui::BeginGroup();
			ImGui::Image(imgui_tex, img_size);
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
					for (const auto& video : ctx_.current_project->videos)
					{
						const auto& info = video.second;
						draw_video_tile(info.path, img_size, tile_size);
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
