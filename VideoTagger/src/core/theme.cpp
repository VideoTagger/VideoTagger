#include "pch.hpp"
#include "theme.hpp"
#include <utils/color.hpp>
#include "app_context.hpp"
#include <events/theme/theme_mode_changed_event.hpp>

#include <ImGuizmo.h>
#include <events/theme/theme_changed_event.hpp>

#ifdef _WIN32
	#include <windows.h>
#endif

namespace vt
{
	static std::unordered_map<theme_color, std::string> color_names
	{
		{ theme_color::accent_light, "accent.light" },
		{ theme_color::accent_medium, "accent.medium" },
		{ theme_color::accent_dark, "accent.dark" },
		{ theme_color::accent_background, "accent.background" },
		{ theme_color::secondary_light, "secondary.light" },
		{ theme_color::secondary_medium, "secondary.medium" },
		{ theme_color::secondary_dark, "secondary.dark" },
		{ theme_color::selection_normal, "selection.normal" },
		{ theme_color::selection_disabled, "selection.disabled" },
		{ theme_color::playhead_normal, "playhead.normal" },
		{ theme_color::playhead_disabled, "playhead.disabled" },
		{ theme_color::axis_x, "axis.x" },
		{ theme_color::axis_y, "axis.y" },
		{ theme_color::axis_z, "axis.z" },
		{ theme_color::icon_thumbnail, "icon.thumbnail" },
		{ theme_color::common_success, "common.success" },
		{ theme_color::common_info, "common.info" },
		{ theme_color::common_warning, "common.warning" },
		{ theme_color::common_error, "common.error" },
		{ theme_color::console_info, "console.info" },
		{ theme_color::console_warning, "console.warning" },
		{ theme_color::console_error, "console.error" },
		{ theme_color::button_normal, "button.normal" },
		{ theme_color::button_hover, "button.hover" },
		{ theme_color::button_active, "button.active" },
		{ theme_color::text_normal, "text.normal" },
		{ theme_color::text_inverted, "text.inverted" },
		{ theme_color::text_disabled, "text.disabled" },
		{ theme_color::background_window, "background.window" },
		{ theme_color::background_child, "background.child" },
		{ theme_color::background_popup, "background.popup" },
		{ theme_color::background_base, "background.base" },
		{ theme_color::background_base_alt, "background.base.alt" },
		{ theme_color::background_secondary, "background.secondary" },
		{ theme_color::background_tertiary, "background.tertiary" },
		{ theme_color::border, "border" },
		{ theme_color::frame_background_normal, "frame.background.normal" },
		{ theme_color::frame_background_hover, "frame.background.hover" },
		{ theme_color::frame_background_active, "frame.background.active" },
		{ theme_color::title_background_normal, "title.background.normal" },
		{ theme_color::title_background_active, "title.background.active" },
		{ theme_color::title_background_collapsed, "title.background.collapsed" },
		{ theme_color::menubar_background, "menubar.background" },
		{ theme_color::scrollbar_background, "scrollbar.background" },
		{ theme_color::scrollbar_grab_normal, "scrollbar.grab.normal" },
		{ theme_color::scrollbar_grab_hover, "scrollbar.grab.hover" },
		{ theme_color::scrollbar_grab_active, "scrollbar.grab.active" },
		{ theme_color::checkmark, "checkmark" },
		{ theme_color::header_normal, "header.normal" },
		{ theme_color::header_hover, "header.hover" },
		{ theme_color::header_active, "header.active" },
		{ theme_color::separator_normal, "separator.normal" },
		{ theme_color::separator_hover, "separator.hover" },
		{ theme_color::separator_active, "separator.active" },
		{ theme_color::tab_focused_normal, "tab.focused.normal" },
		{ theme_color::tab_focused_hover, "tab.focused.hover" },
		{ theme_color::tab_focused_active, "tab.focused.active" },
		{ theme_color::tab_unfocused_normal, "tab.unfocused.normal" },
		{ theme_color::tab_unfocused_active, "tab.unfocused.active" },
	};

	static std::unordered_map<ImGuiCol, theme_color> imgui_color_map
	{
		{ ImGuiCol_Text, theme_color::text_normal },
		{ ImGuiCol_TextDisabled, theme_color::text_disabled },
		{ ImGuiCol_WindowBg, theme_color::background_window },
		{ ImGuiCol_ChildBg, theme_color::background_child },
		{ ImGuiCol_PopupBg, theme_color::background_popup },
		{ ImGuiCol_Border, theme_color::border },
		{ ImGuiCol_FrameBg, theme_color::frame_background_normal },
		{ ImGuiCol_FrameBgHovered, theme_color::frame_background_hover },
		{ ImGuiCol_FrameBgActive, theme_color::frame_background_active },
		{ ImGuiCol_TitleBg, theme_color::title_background_normal },
		{ ImGuiCol_TitleBgActive, theme_color::title_background_active },
		{ ImGuiCol_TitleBgCollapsed, theme_color::title_background_collapsed },
		{ ImGuiCol_MenuBarBg, theme_color::menubar_background },
		{ ImGuiCol_ScrollbarBg, theme_color::scrollbar_background },
		{ ImGuiCol_ScrollbarGrab, theme_color::scrollbar_grab_normal },
		{ ImGuiCol_ScrollbarGrabHovered, theme_color::scrollbar_grab_hover },
		{ ImGuiCol_ScrollbarGrabActive, theme_color::scrollbar_grab_active },
		{ ImGuiCol_CheckMark, theme_color::checkmark },
		{ ImGuiCol_Button, theme_color::button_normal },
		{ ImGuiCol_ButtonHovered, theme_color::button_hover },
		{ ImGuiCol_ButtonActive, theme_color::button_active },
		{ ImGuiCol_Header, theme_color::header_normal },
		{ ImGuiCol_HeaderHovered, theme_color::header_hover },
		{ ImGuiCol_HeaderActive, theme_color::header_active },
		{ ImGuiCol_Separator, theme_color::separator_normal },
		{ ImGuiCol_SeparatorHovered, theme_color::separator_hover },
		{ ImGuiCol_SeparatorActive, theme_color::separator_active },
		{ ImGuiCol_Tab, theme_color::tab_focused_normal },
		{ ImGuiCol_TabHovered, theme_color::tab_focused_hover },
		{ ImGuiCol_TabActive, theme_color::tab_focused_active },
		{ ImGuiCol_TabUnfocused, theme_color::tab_unfocused_normal },
		{ ImGuiCol_TabUnfocusedActive, theme_color::tab_unfocused_active },
	};

	void theme::apply()
	{
		auto& style = ImGui::GetStyle();
		event_source event_source_{ "theme" };
		for (auto& [imgui_col, theme_col] : imgui_color_map)
		{
			style.Colors[imgui_col] = get_float4(theme_col);
		}

		auto& gizmo_style = ImGuizmo::GetStyle();
		gizmo_style.Colors[ImGuizmo::COLOR::DIRECTION_X] = get_float4(theme_color::axis_x);
		gizmo_style.Colors[ImGuizmo::COLOR::DIRECTION_Y] = get_float4(theme_color::axis_y);
		gizmo_style.Colors[ImGuizmo::COLOR::PLANE_Z] = get_float4(theme_color::axis_z);
		gizmo_style.Colors[ImGuizmo::COLOR::SELECTION] = get_float4(theme_color::selection_normal);

		ctx_.dispatch_event<theme_changed_event>(event_source_, *this);
		ctx_.dispatch_event<theme_mode_changed_event>(event_source_, *this, is_dark_);
	}

	void theme::push_color(theme_color color, uint32_t rgba)
	{
		color_overrides_.push({ color, get_rgba(color) });
		set_color(color, rgba);
	}

	void theme::push_color(theme_color color, ImVec4 rgba)
	{
		push_color(color, ImGui::ColorConvertFloat4ToU32(rgba));
	}

	void theme::pop_color(size_t count)
	{
		while (count-- > 0 && !color_overrides_.empty())
		{
			auto& override = color_overrides_.top();
			set_color(override.color, override.previous_value);
			color_overrides_.pop();
		}
	}

	void theme::set_name(const std::string& name)
	{
		name_ = name;
	}

	void theme::set_dark(bool is_dark)
	{
		is_dark_ = is_dark;
	}

	void theme::set_color(theme_color name, uint32_t rgba)
	{
		colors_[name] = rgba;
	}

	void theme::set_color(theme_color name, ImVec4 color)
	{
		colors_[name] = ImGui::ColorConvertFloat4ToU32(color);
	}

	const std::string& theme::name() const
	{
		return name_;
	}

	bool theme::is_dark() const
	{
		return is_dark_;
	}

	uint32_t theme::get_rgba(theme_color name) const
	{
		return colors_.at(name);
	}

	uint32_t theme::get_rgba(const std::string& name) const
	{
		auto opt_col = to_theme_color(name);
		if (!opt_col.has_value()) return 0;
		return get_rgba(opt_col.value());
	}

	ImVec4 theme::get_float4(theme_color name) const
	{
		return ImGui::ColorConvertU32ToFloat4(get_rgba(name));
	}

	ImVec4 theme::get_float4(const std::string& name) const
	{
		return ImGui::ColorConvertU32ToFloat4(get_rgba(name));
	}

	void theme::save(const std::filesystem::path& filepath) const
	{
		nlohmann::ordered_json json;
		auto& colors = json["colors"];
		for (auto& [key, col] : colors_)
		{
			colors[to_string(key)] = utils::color::to_string(col, false);
		}
		utils::json::write_to_file(json, filepath);
	}

	theme theme::load_from_json(const nlohmann::ordered_json& json)
	{
		theme result;
		if (json.contains("@meta"))
		{
			auto& meta = json["@meta"];
			if (meta.contains("name"))
			{
				result.set_name(meta["name"].get<std::string>());
			}
			if (meta.contains("dark"))
			{
				result.set_dark(meta["dark"].get<bool>());
			}
		}
		if (json.contains("colors"))
		{
			const auto& colors = json["colors"];
			for (auto& [key, json_col] : colors.items())
			{
				uint32_t color{};
				if (utils::color::parse_string(json_col.get<std::string>(), color, false))
				{
					auto opt_col = to_theme_color(key);
					if (!opt_col.has_value()) continue;
					result.set_color(opt_col.value(), color);
				}
			}
		}
		return result;
	}

	theme theme::load_from_file(const std::filesystem::path& filepath)
	{
		return load_from_json(utils::json::load_from_file(filepath));
	}

	std::optional<theme_color> theme::to_theme_color(const std::string& name)
	{
		auto it = std::find_if(color_names.begin(), color_names.end(), [&name](const auto& pair)
		{
			return pair.second == name;
		});
		if (it != color_names.end()) return it->first;
		return std::nullopt;
	}

	std::string theme::to_string(theme_color color)
	{
		auto it = color_names.find(color);
		if (it != color_names.end()) return it->second;
		return {};
	}

	bool theme::system_uses_dark_mode()
	{
#ifdef _WIN32
		using ShouldAppsUseDarkModeFn = bool (*)();
		HMODULE handle = LoadLibraryA("uxtheme.dll");
		if (!handle) return false;

		auto fn = (ShouldAppsUseDarkModeFn)GetProcAddress(handle, MAKEINTRESOURCEA(132));
		if (!fn) return false;

		return fn();
#else
		//TODO: Add support for other systems
		return true;
#endif
	}
}
