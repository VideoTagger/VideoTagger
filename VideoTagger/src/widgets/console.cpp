#include "pch.hpp"
#include "console.hpp"
#include "icons.hpp"
#include "controls.hpp"
#include <core/debug.hpp>
#include <utils/string.hpp>

namespace vt::widgets
{
	static uint32_t flag_color(console::entry::flag_type flag)
	{
		switch (flag)
		{
			case console::entry::flag_type::error: return 0xFF6758F2/*0xFF0000C2*/;
			case console::entry::flag_type::warn: return 0xFF94DFFF/*0xFF00B6FF*/;
			//case console::entry::flag_type::info: break;
			default: return 0xFFFEB95D/*0xFFFF9400*/;
		}
	}

	static const char* flag_text(console::entry::flag_type flag)
	{
		switch (flag)
		{
			case console::entry::flag_type::error: return "Error";
			case console::entry::flag_type::warn: return "Warn";
				//case console::entry::flag_type::info: break;
			default: return "Info";
		}
	}

	static const char* flag_icon(console::entry::flag_type flag)
	{
		switch (flag)
		{
			case console::entry::flag_type::error: return icons::error;
			case console::entry::flag_type::warn: return icons::warning;
				//case console::entry::flag_type::info: break;
			default: return icons::info;
		}
	}

	void console::render(bool& is_open, bool& clear_on_run, const std::filesystem::path& scripts_path)
	{
		const auto& style = ImGui::GetStyle();
		bool win_open = ImGui::Begin(window_name().c_str(), &is_open);

		if (win_open)
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
			auto avail_width = ImGui::GetContentRegionAvail().x;

			size_t infos{};
			size_t warns{};
			size_t errors{};

			if (ImGui::IsWindowAppearing())
			{
				filter.clear();
			}

			decltype(entries_) filtered_entries;
			filtered_entries.reserve(entries_.size());
			for (const auto& entry : entries_)
			{
				if (!filter.empty() and (utils::string::to_lowercase(entry.message).find(filter) == std::string::npos and std::filesystem::relative(entry.info->path, scripts_path).string().find(filter) == std::string::npos)) continue;

				switch (entry.flag)
				{
					case console::entry::flag_type::info:
					{
						if (show_infos)
						{
							filtered_entries.push_back(entry);
							++infos;
						}
					}
					break;
					case console::entry::flag_type::warn:
					{
						if (show_warns)
						{
							filtered_entries.push_back(entry);
							++warns;
						}
					}
					break;
					case console::entry::flag_type::error:
					{
						if (show_errors)
						{
							filtered_entries.push_back(entry);
							++errors;
						}
					}
					break;
				}
			}

			bool has_entries = !filtered_entries.empty();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (icon_toggle_button(fmt::format("{}{}", icons::error, errors).c_str(), show_errors, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::error))))
			{
				show_errors = !show_errors;
			}
			tooltip(show_errors ? "Error: On" : "Error: Off");

			ImGui::SameLine();
			if (icon_toggle_button(fmt::format("{}{}", icons::warning, warns).c_str(), show_warns, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::warn))))
			{
				show_warns = !show_warns;
			}
			tooltip(show_warns ? "Warn: On" : "Warn: Off");

			ImGui::SameLine();
			if (icon_toggle_button(fmt::format("{}{}", icons::info, infos).c_str(), show_infos, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::info))))
			{
				show_infos = !show_infos;
			}
			tooltip(show_infos ? "Info: On" : "Info: Off");

			ImGui::SameLine();
			if (icon_button(icons::delete_))
			{
				clear();
			}
			tooltip("Clear");
			ImGui::SameLine();
			if (icon_toggle_button(icons::delete_on_run, clear_on_run))
			{
				clear_on_run = !clear_on_run;
			}
			tooltip(clear_on_run ? "Clear On Run: On" : "Clear On Run: Off");

			ImGui::PopStyleVar();
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();

			if (search_bar("##ConsoleMessageFilter", "Filter...", filter))
			{
				filter = utils::string::to_lowercase(utils::string::trim_whitespace(filter));
			}

			ImGui::Separator();
			bool table_open = has_entries and ImGui::BeginTable("##ConsoleEntries", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, { avail_width, 0.f });

			if (!has_entries)
			{
				auto avail = ImGui::GetContentRegionAvail();
				centered_text("Nothing to display...", avail, ImVec2{ 0.f, ImGui::GetCursorPosY() });
			}

			if (table_open)
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::CalcTextSize(icons::error).x);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoDirectResize_, 0.25f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoDirectResize_, 0.70f);
				ImGui::TableSetColumnWidthAutoSingle(ImGui::GetCurrentTable(), 1);

				int i{};
				for (const auto& entry : filtered_entries)
				{
					ImGui::PushID(i++);
					ImGui::TableNextRow();
					bool is_hovered = table_hovered_row_style();
					if (!is_hovered)
					{
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_MenuBarBg]));
					}

					ImGui::TableNextColumn();
					//color_indicator(3.f, flag_color(entry.flag));
					//ImGui::SameLine(2 * style.ItemSpacing.x + 3.f);
					ImGui::AlignTextToFramePadding();
					ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(flag_color(entry.flag)), "%s", flag_icon(entry.flag));
					
					/*ImGui::SameLine();
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted(flag_text(entry.flag));*/

					ImGui::TableNextColumn();
					if (entry.info.has_value())
					{
						std::string path;
						bool exists = std::filesystem::exists(entry.info->path);
						if (entry.info->line > 0)
						{
							path = fmt::format("{}:{}", std::filesystem::relative(entry.info->path, scripts_path).string(), entry.info->line);
						}
						else
						{
							path = fmt::format("{}", exists ? std::filesystem::relative(entry.info->path, scripts_path).string() : entry.info->path.string());
						}
						auto abs_path = entry.info->path.string();
						ImGui::TextUnformatted(path.c_str());
						if (exists)
						{
							if (ImGui::IsItemHovered())
							{
								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
							}
							if (ImGui::IsItemClicked(0))
							{
								SDL_SetClipboardText(abs_path.c_str());
							}
							tooltip(abs_path.c_str());
						}
					}
					
					ImGui::TableNextColumn();
					ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
					ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
					ImGui::InputTextMultiline("##ConsoleEntryMessage", const_cast<std::string*>(&entry.message), ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::CalcTextSize(entry.message.c_str()).y + 2 * style.FramePadding.y }, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopStyleColor(2);
					if (ImGui::IsItemHovered())
					{
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt)));
					}
					if (ImGui::BeginPopupContextItem("##ConsoleEntryMessageCtxMenu"))
					{
						if (ImGui::MenuItem("Copy"))
						{
							SDL_SetClipboardText(entry.message.c_str());
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	void console::add_entry(entry::flag_type flag, const std::string& message, const std::optional<entry::source_info>& info)
	{
		entries_.push_back({ flag, message, info });
	}

	void console::clear()
	{
		entries_.clear();
	}

	std::string console::window_name()
	{
		return fmt::format("{} Console###Console", icons::terminal);
	}
}
