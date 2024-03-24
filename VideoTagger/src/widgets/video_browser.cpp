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

		static auto draw_video_tile = [this](const video_pool::video_metadata& vmeta, ImVec2 image_tile_size, ImVec2 tile_size, bool& open, bool& remove, SDL_Texture* texture = nullptr)
		{
			int video_width = vmeta.width;
			int video_height = vmeta.height;

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
			std::string name = vmeta.path.stem().u8string();
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
				if (!vmeta.is_widget_open and ImGui::MenuItem("Open"))
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

		static auto draw_group_tile = [this](const video_group& vgroup, video_group_id_t gid, ImVec2 image_tile_size, ImVec2 tile_size, bool& open_group, bool& remove_group)
		{
			ImVec2 image_size = image_tile_size;

			auto& style = ImGui::GetStyle();
			std::string name = std::to_string(gid);
			ImGui::TableNextColumn();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			ImTextureID imgui_tex = static_cast<ImTextureID>(nullptr);
			const char* id = name.c_str();
			ImGui::PushID(id);
			bool selected = false;
			auto text_size = ImVec2{ 0, ImGui::CalcTextSize(id, nullptr, false, tile_size.x).y };
			auto selectable_size = tile_size + style.FramePadding + text_size;
			ImVec2 cpos = ImGui::GetCursorPos() + (selectable_size - image_size - text_size) / 2;
			if (ImGui::Selectable("##GroupTileButton", &selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick, selectable_size))
			{

			}
			if (ImGui::IsItemActivated() and ImGui::IsMouseDoubleClicked(0))
			{
				open_group = true;
			}
			if (ImGui::BeginPopupContextItem("##GroupTileCtxMenu"))
			{
				if (!ImGui::MenuItem("Open"))
				{
					open_group = true;
				}
				if (ImGui::MenuItem("Remove"))
				{
					remove_group = true;
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

			ImVec2 img_tile_size{ thumbnail_size, thumbnail_size };
			img_tile_size.x -= text_height;
			img_tile_size.y -= text_height;

			ImVec2 tile_size = img_tile_size + style.ItemSpacing + style.CellPadding / 2;
			auto avail = ImGui::GetContentRegionAvail() - ImVec2{ 0, ImGui::GetTextLineHeightWithSpacing() };
			int columns = static_cast<int>(avail.x / tile_size.x);
			if (columns < 1)
			{
				columns = 1;
			}

			if (ctx_.current_project->videos.size() > 0)
			{
				static auto draw_group_tab = [&style](const std::string& group_name, video_group_id_t gid)
				{
					bool inactive = ctx_.active_video_group_id != gid;

					if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
					if (ImGui::TreeNodeEx(group_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf) and ImGui::IsItemClicked())
					{
						ctx_.active_video_group_id = gid;
					}
					if (inactive) ImGui::PopStyleColor();
				};

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
						if (ctx_.active_video_group_id == 0)
						{
							for (const auto& [gid, group] : ctx_.current_project->video_groups)
							{
								bool open_group = false;
								bool remove_group = false;

								draw_group_tile(group, gid, img_tile_size, tile_size, open_group, remove_group);
								if (remove_group)
								{
									ctx_.current_project->video_groups.erase(gid);
									break;
								}

								if (open_group)
								{
									debug::log("Opening group " + std::to_string(gid));
									ctx_.active_video_group_id = gid;
								}
							}
						}
						else
						{
							auto& pool = ctx_.current_project->videos;
							auto& vgroup = ctx_.current_project->video_groups.at(ctx_.active_video_group_id);
							for (auto& vinfo : vgroup)
							{
								auto metadata = pool.get(vinfo.id);
								if (metadata == nullptr) continue;

								bool open_video{};
								bool remove_video{};
								draw_video_tile(*metadata, img_tile_size, tile_size, open_video, remove_video, metadata->thumbnail);
								if (remove_video)
								{
									ctx_.current_project->videos.erase(vinfo.id);
									break;
								}

								if (open_video and !metadata->is_widget_open)
								{
									debug::log("Opening video " + metadata->path.string());
									if (on_open_video != nullptr)
									{
										std::invoke(on_open_video, vinfo.id);
									}
								}

								if (ImGui::IsWindowFocused() and ImGui::IsKeyPressed(ImGuiKey_Escape))
								{
									ctx_.active_video_group_id = 0;
								}
							}
						}
						ImGui::EndTable();
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
