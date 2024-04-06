#include "pch.hpp"
#include "theme.hpp"
#include <utils/json.hpp>
#include <utils/color.hpp>

namespace vt
{
	static std::unordered_map<ImGuiCol_, std::string> color_names
	{
		{ ImGuiCol_WindowBg, "window.background" },
		{ ImGuiCol_ChildBg, "child.background" },
		{ ImGuiCol_PopupBg, "popup.background" },
		{ ImGuiCol_Border, "border" },
		{ ImGuiCol_MenuBarBg, "menubar.background" },
		{ ImGuiCol_FrameBg, "frame.background.normal" },
		{ ImGuiCol_FrameBgHovered, "frame.background.hovered" },
		{ ImGuiCol_FrameBgActive, "frame.background.active" },
		{ ImGuiCol_TitleBg, "title.background.normal" },
		{ ImGuiCol_TitleBgActive, "title.background.active" },
		{ ImGuiCol_TitleBgCollapsed, "title.background.collapsed" },
		{ ImGuiCol_Text, "text.normal" },
		{ ImGuiCol_TextDisabled, "text.disabled" },
		{ ImGuiCol_Button, "button.normal" },
		{ ImGuiCol_ButtonHovered, "button.hovered" },
		{ ImGuiCol_ButtonActive, "button.active" },
		{ ImGuiCol_CheckMark, "checkmark" },
		{ ImGuiCol_Tab, "tab.focused.normal" },
		{ ImGuiCol_TabHovered, "tab.focused.hovered" },
		{ ImGuiCol_TabActive, "tab.focused.active" },
		{ ImGuiCol_TabUnfocused, "tab.unfocused.normal" },
		{ ImGuiCol_TabUnfocusedActive, "tab.unfocused.active" },
		{ ImGuiCol_ScrollbarBg, "scrollbar.background" },
		{ ImGuiCol_ScrollbarGrab, "scrollbar.grab.normal" },
		{ ImGuiCol_ScrollbarGrabHovered, "scrollbar.grab.hovered" },
		{ ImGuiCol_ScrollbarGrabActive, "scrollbar.grab.active" },
		{ ImGuiCol_Header, "header.normal" },
		{ ImGuiCol_HeaderHovered, "header.hovered" },
		{ ImGuiCol_HeaderActive, "header.active" },
		{ ImGuiCol_Separator, "separator.normal" },
		{ ImGuiCol_SeparatorHovered, "separator.hovered" },
		{ ImGuiCol_SeparatorActive, "separator.active" }
	};

	void theme::save(const std::filesystem::path& filepath) const
	{
		nlohmann::ordered_json json;
		auto& colors = json["colors"];
		for (size_t i = 0; i < ImGuiCol_COUNT; ++i)
		{
			auto it = color_names.find(static_cast<ImGuiCol_>(i));
			if (it == color_names.end()) continue;
			colors[it->second] = utils::color::to_string(ImGui::GetColorU32(style.Colors[i]), false);
		}
		utils::json::write_to_file(json, filepath);
	}

	theme theme::load_from_file(const std::filesystem::path& filepath)
	{
		theme result;
		auto json = utils::json::load_from_file(filepath);
		if (json.contains("colors"))
		{
			//TODO: This is temporary and shouldnt be done in the future
			result.style = ImGui::GetStyle();
			const auto& colors = json["colors"];
			for (size_t i = 0; i < ImGuiCol_COUNT; ++i)
			{
				auto it = color_names.find(static_cast<ImGuiCol_>(i));
				if (it == color_names.end()) continue;
				if (colors.contains(it->second))
				{
					uint32_t color{};
					if (utils::color::parse_string(colors[it->second], color, false))
					{
						result.style.Colors[it->first] = ImGui::ColorConvertU32ToFloat4(color);
					}
				}
			}
		}
		return result;
	}
}
