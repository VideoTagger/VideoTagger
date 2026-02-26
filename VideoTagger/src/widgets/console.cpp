#include "pch.hpp"
#include "console.hpp"
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include "controls.hpp"
#include <core/debug.hpp>
#include <utils/string.hpp>
#include <core/app_context.hpp>

namespace vt::widgets
{
	console::console() : ui::window{ "Console", "console", "Console" }
	{
		set_icon(icons::terminal);
	}

	static uint32_t flag_color(console::entry::flag_type flag)
	{
		switch (flag)
		{
			case console::entry::flag_type::error: return ctx_.current_theme.get_rgba(theme_color::console_error);
			case console::entry::flag_type::warn: return ctx_.current_theme.get_rgba(theme_color::console_warning);
			default: return ctx_.current_theme.get_rgba(theme_color::console_info);
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

	void console::on_render()
	{
		const auto& style = ImGui::GetStyle();

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
			if (!filter.empty() and (utils::string::to_lowercase(entry.message).find(filter) == std::string::npos and std::filesystem::relative(entry.info->path, scripts_path_).string().find(filter) == std::string::npos)) continue;

			switch (entry.flag)
			{
				case console::entry::flag_type::info:
				{
					if (show_infos_)
					{
						filtered_entries.push_back(entry);
						++infos;
					}
				}
				break;
				case console::entry::flag_type::warn:
				{
					if (show_warns_)
					{
						filtered_entries.push_back(entry);
						++warns;
					}
				}
				break;
				case console::entry::flag_type::error:
				{
					if (show_errors_)
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
		if (ui::icon_toggle_button(fmt::format("{}{}", icons::error, errors), show_errors_, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::error))))
		{
			show_errors_ = !show_errors_;
		}
		ui::tooltip(show_errors_ ? "Error: On" : "Error: Off");

		ImGui::SameLine();
		if (ui::icon_toggle_button(fmt::format("{}{}", icons::warning, warns), show_warns_, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::warn))))
		{
			show_warns_ = !show_warns_;
		}
		ui::tooltip(show_warns_ ? "Warn: On" : "Warn: Off");

		ImGui::SameLine();
		if (ui::icon_toggle_button(fmt::format("{}{}", icons::info, infos), show_infos_, {}, ImGui::ColorConvertU32ToFloat4(flag_color(console::entry::flag_type::info))))
		{
			show_infos_ = !show_infos_;
		}
		ui::tooltip(show_infos_ ? "Info: On" : "Info: Off");

		ImGui::SameLine();
		if (ui::icon_button(icons::delete_))
		{
			clear();
		}
		ui::tooltip("Clear");
		ImGui::SameLine();
		if (ui::icon_toggle_button(icons::delete_on_run, clear_on_run_))
		{
			clear_on_run_ = !clear_on_run_;
		}
		ui::tooltip(clear_on_run_ ? "Clear On Run: On" : "Clear On Run: Off");

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
			ui::centered_text("Nothing to display...", avail, ImVec2{ 0.f, ImGui::GetCursorPosY() });
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
						path = fmt::format("{}:{}", std::filesystem::relative(entry.info->path, scripts_path_).string(), entry.info->line);
					}
					else
					{
						path = fmt::format("{}", exists ? std::filesystem::relative(entry.info->path, scripts_path_).string() : entry.info->path.string());
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
						ui::tooltip(abs_path);
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

	nlohmann::ordered_json console::serialize() const
	{
		nlohmann::ordered_json json;
		json["clear-on-run"] = clear_on_run_;
		return json;
	}

	void console::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("clear-on-run") and json["clear-on-run"].is_boolean())
		{
			clear_on_run_ = json["clear-on-run"].get<bool>();
		}
	}

	void console::on_run_script()
	{
		if (clear_on_run_)
		{
			clear();
		}
	}

	void console::set_scripts_path(const std::filesystem::path& path)
	{
		scripts_path_ = path;
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
