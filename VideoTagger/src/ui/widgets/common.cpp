#include "pch.hpp"
#include "common.hpp"

#include <imgui_toggle.h>
#include <imgui_toggle_palette.h>

#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	void tooltip(const std::string& text)
	{
		if (text.empty()) return;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f);

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal) and ImGui::BeginTooltip())
		{
			ImGui::TextUnformatted(text.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();
	}

	void tooltip(const std::string& text, ImVec2 pos)
	{
		if (text.empty()) return;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f);

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayNormal))
		{
			ImGui::SetNextWindowPos(pos);
			if (ImGui::BeginTooltip())
			{
				ImGui::TextUnformatted(text.c_str());
				ImGui::EndTooltip();
			}
		}
		ImGui::PopStyleVar();
	}

	void help_marker(const std::string& description)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
		ImGui::TextDisabled(icons::help);
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(description.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();
	}

	bool is_scrollbar_hovered()
	{
		ImGuiContext& g = *ImGui::GetCurrentContext();
		auto window = g.CurrentWindow;

		bool hovered = false;
		bool visible = false;
		if (window->ScrollbarX)
		{
			visible = true;
			hovered |= ImGui::GetWindowScrollbarID(window, ImGuiAxis_X) == ImGui::GetHoveredID();
		}

		if (window->ScrollbarY)
		{
			visible = true;
			hovered |= ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y) == ImGui::GetHoveredID();
		}
		return hovered;
	}

	static inline std::unordered_map<std::string, bool> scrollbar_hover_state{};

	void begin_styled_scrollbars(const std::string& window_id)
	{
		const auto& style = ImGui::GetStyle();
		const auto& theme = ctx_.current_theme;
		auto it = scrollbar_hover_state.find(window_id);
		bool hovered = false;
		if (it != scrollbar_hover_state.end())
		{
			hovered = it->second;
		}

		auto color = style.Colors[ImGuiCol_ScrollbarBg];
		color.w *= 0.25f;
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, color);

		if (hovered)
		{
			ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, style.Colors[ImGuiCol_ScrollbarBg]);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, 0);
		}
	}

	void save_window_scrollbar_state(const std::string& window_id)
	{
		ImGuiContext& g = *ImGui::GetCurrentContext();
		auto window = g.CurrentWindow;
		scrollbar_hover_state[window_id] = false;
		/*

		if (window->ScrollbarX)
		{
			scrollbar_hover_state[window_id] |= ImGui::GetWindowScrollbarID(window, ImGuiAxis_X) == ImGui::GetHoveredID();
		}
		if (window->ScrollbarY)
		{
			scrollbar_hover_state[window_id] |= ImGui::GetWindowScrollbarID(window, ImGuiAxis_Y) == ImGui::GetHoveredID();
		}

		if (scrollbar_hover_state[window_id])
		{
			auto a = 1;
		}
		*/
		scrollbar_hover_state[window_id] = ImGui::IsWindowHovered();
	}

	void end_styled_scrollbars()
	{
		ImGui::PopStyleColor(2);
	}

	void begin_bigger_frames()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, bigger_frame_size());
	}

	void end_bigger_frames()
	{
		ImGui::PopStyleVar();
	}

    void begin_modal_style()
    {
		const auto& style = ImGui::GetStyle();
		begin_rounded_window_style();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding * 2);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

	void end_modal_style()
	{
		end_rounded_window_style();
		ImGui::PopStyleVar();
	}

	void begin_rounded_window_style()
	{
		const auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
	}

	void end_rounded_window_style()
	{
		ImGui::PopStyleVar();
	}

	void begin_rounded_popup_style()
	{
		const auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.f);
	}

	void end_rounded_popup_style()
	{
		ImGui::PopStyleVar();
	}

	void label(const std::string& label)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, {});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, ImGui::GetStyle().FramePadding.y });
		ImGui::Button(label.c_str());
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
	}

	void centered_text(const std::string& text, ImVec2 avail_area, ImVec2 offset)
	{
		//auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
		auto half_text_size = ImGui::CalcTextSize(text.c_str(), nullptr, false, 3 * avail_area.x / 4) / 2;
		auto cpos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(offset + avail_area / 2 - half_text_size);
		ImGui::BeginDisabled();
		ImGui::TextWrapped("%s", text.c_str());
		ImGui::EndDisabled();
		ImGui::SetCursorPos(cpos);
	}

	void clipped_text(const std::string& text, ImVec2 avail_area)
	{
		static auto get_text_size = [&](const std::string& text)
		{
			auto size = ImGui::CalcTextSize(text.c_str(), nullptr, false, avail_area.x);
			return size;
		};

		ImVec2 text_size = get_text_size(text);

		std::string str = text;
		if (text_size.x <= avail_area.x and text_size.y <= avail_area.y)
		{
			ImGui::TextWrapped("%s", text.c_str());
			return;
		}

		while (!str.empty() and (text_size.x > avail_area.x or text_size.y > avail_area.y))
		{
			str.pop_back();
			std::string temp = str + "...";
			text_size = get_text_size(temp.c_str());
		}

		if (!str.empty())
		{
			std::string temp = str + "...";
			ImGui::TextWrapped("%s", temp.c_str());
		}
	}

	void text_with_size(const std::string& text, ImVec2 size)
	{
		auto text_cstr = text.c_str();
		auto text_size = ImGui::CalcTextSize(text_cstr);
		size.x = std::max(size.x, text_size.x);
		size.y = std::max(size.y, text_size.y);

		ImGui::SetCursorPos(ImGui::GetCursorPos() + (size - text_size) / 2);
		ImGui::TextUnformatted(text_cstr);
	}

	void item_spacer(const ImVec2& size)
	{
		const auto& style = ImGui::GetStyle();
		ImGui::Dummy({ size.x == 0.f ? style.ItemSpacing.x : size.x, size.y == 0.f ? style.ItemSpacing.y : size.y });
	}

	void vertical_item_spacer(float height)
	{
		const auto& style = ImGui::GetStyle();
		ImGui::Dummy({ 0.f, height == 0.f ? style.ItemSpacing.y : height });
	}

	void horizontal_item_spacer(float width)
	{
		const auto& style = ImGui::GetStyle();
		ImGui::Dummy({ width == 0.f ? style.ItemSpacing.x : width, 0.f });
	}

	bool rounded_button(const std::string& label, const ImVec2& size)
	{
		const auto& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
		bool result = ImGui::Button(label.c_str(), size);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		ImGui::PopStyleVar();
		return result;
	}

	bool icon_button(const std::string& label, const ImVec2& size, const ImVec4& color)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);
		bool result = ImGui::Button(label.c_str(), size);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
		if (!is_item_disabled() and ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		return result;
	}

	bool icon_button_no_cursor(const std::string& label, const ImVec2& size, const ImVec4& color)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);
		bool result = ImGui::Button(label.c_str(), size);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		return result;
	}

	bool icon_toggle_button(const std::string& label, bool is_toggled, const ImVec2& size, const ImVec4& color)
	{
		bool result = icon_button(label, size, is_toggled ? color : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		if (!is_item_disabled() and ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		return result;
	}

	bool icon_toggle_button(const std::string& label_on, const std::string& label_off, bool is_toggled, const ImVec2& size, const ImVec4& color)
	{
		return icon_toggle_button(is_toggled ? label_on : label_off, is_toggled, size, color);
	}

	bool accent_button(const std::string& label, const ImVec2& size)
	{
		const auto& theme = ctx_.current_theme;

		auto btn_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		auto btn_hov_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		auto btn_actv_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		//TODO: Move these colors into theme class
		bool is_disabled = ui::is_item_disabled();
		ImGui::PushStyleColor(ImGuiCol_Button, is_disabled ? theme.get_float4(theme_color::secondary_light) : theme.get_float4(theme_color::accent_light)); // push button color if diabled or accent color if enabled
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.get_float4(theme_color::accent_medium));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.get_float4(theme_color::accent_dark));

		//auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		//text_color.x = 1.f - text_color.x;
		//text_color.y = 1.f - text_color.y;
		//text_color.z = 1.f - text_color.z;
		auto text_color = theme.get_float4(theme_color::text_inverted);
		if (!is_disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, text_color);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		}
		bool result = rounded_button(label, size);
		ImGui::PopStyleColor(4);
		return result;
	}

	bool button(const std::string& label, const ImVec2& size)
	{
		const auto& theme = ctx_.current_theme;
		auto btn_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		auto btn_hov_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		auto btn_actv_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		ImGui::PushStyleColor(ImGuiCol_Button, theme.get_float4(theme_color::secondary_light));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme.get_float4(theme_color::secondary_medium));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme.get_float4(theme_color::secondary_dark));

		bool result = rounded_button(label, size);
		ImGui::PopStyleColor(3);
		return result;
	}

	bool color_button(const std::string& id, const ImVec4& color, ImGuiColorEditFlags flags, const ImVec2& size)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.f);
		auto value = ImGui::ColorButton(id.c_str(), color, flags, size);
		ImGui::PopStyleVar();
		return value;
	}

	bool color_edit3(const std::string& label, ImVec4& color, ImGuiColorEditFlags flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.f);
		auto value = ImGui::ColorEdit3(label.c_str(), (float*)&color, flags);
		ImGui::PopStyleVar();
		return value;
	}

	bool color_edit4(const std::string& label, ImVec4& color, ImGuiColorEditFlags flags)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.f);
		auto value = ImGui::ColorEdit4(label.c_str(), (float*)&color, flags);
		ImGui::PopStyleVar();
		return value;
	}

	static ImGuiToggleConfig toggle_config()
	{
		float size_scale = 1.f;
		const auto& theme = ctx_.current_theme;

		const ImVec4 color_default = theme.get_float4(theme_color::accent_light);
		const ImVec4 color_hover = theme.get_float4(theme_color::accent_medium);
		const ImVec4 color_dim = theme.get_float4(theme_color::accent_dark);

		const ImVec2 material_size(37 * size_scale, 16 * size_scale);
		const float material_inset = -2.5f * size_scale;

		auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		auto text_color_dim = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		auto text_color_inverted = theme.get_float4(theme_color::text_inverted);

		static ImGuiTogglePalette palette_on;
		palette_on.Frame = color_default;
		palette_on.FrameHover = color_dim;
		palette_on.Knob = text_color_inverted;
		palette_on.KnobHover = text_color_inverted;

		static ImGuiTogglePalette palette_off;
		//palette_off.Frame = color_dim;
		//palette_off.FrameHover = color_dim;
		palette_off.FrameBorder = text_color_dim;
		palette_off.Knob = text_color_dim;
		palette_off.KnobHover = text_color;

		// setup config
		ImGuiToggleConfig config;
		config.Flags |= ImGuiToggleFlags_Animated | ImGuiToggleFlags_BorderedFrame;
		config.Size = { 0.f, toggle_height() };
		config.WidthRatio = ImGuiToggleConstants::WidthRatioDefault * 1.25f;
		config.On.KnobInset = config.Off.KnobInset = 4.f * size_scale;
		config.On.KnobOffset = config.Off.KnobOffset = ImVec2(size_scale * 0.9f, 0);
		config.On.Palette = &palette_on;
		config.On.FrameBorderThickness = 0.f;
		config.Off.FrameBorderThickness = 0.5f;
		config.Off.Palette = &palette_off;
		return config;
	}

    bool toggle(const std::string& label, bool& value)
    {
		auto config = toggle_config();
		bool result = ImGui::Toggle(label.c_str(), &value, config);
		if (!is_item_disabled() and ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		return result;
    }

	bool checkbox(const std::string& label, bool& value)
	{
		auto& style = ImGui::GetStyle();
		const auto& theme = ctx_.current_theme;

		bool result{};
		bool is_disabled = ui::is_item_disabled();
		if (value)
		{
			ImGui::PushStyleColor(ImGuiCol_CheckMark, theme.get_float4(theme_color::text_inverted));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, is_disabled ? theme.get_float4(theme_color::secondary_light) : theme.get_float4(theme_color::accent_light));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, theme.get_float4(theme_color::accent_medium));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, theme.get_float4(theme_color::accent_dark));
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.FramePadding.x, style.FramePadding.y } / 3.f);
		bool last_value = value;
		result = ImGui::Checkbox(label.c_str(), &value);
		ImGui::PopStyleVar(2);
		if (last_value)
		{
			ImGui::PopStyleColor(4);
		}
		return result;
	}

	bool collapsing_header(const std::string& label, bool hide_background)
	{
		auto& style = ImGui::GetStyle();
		if (hide_background) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
		std::string node_id = "##Node" + std::string(label);
		auto cx = ImGui::GetCursorPosX();
		bool result = ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		ImGui::PopStyleVar();

		int pop_count = 1;
		if (hide_background) ++pop_count;
		ImGui::PopStyleColor(pop_count);
		auto icon = result ? icons::expand_less : icons::expand_more;

		ImGui::SameLine();
		auto px = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(px - (px - cx) + style.ItemInnerSpacing.x);
		//ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine(ImGui::GetContentRegionMax().x - style.FramePadding.x - ImGui::CalcTextSize(icon).x);
		ImGui::TextUnformatted(icon);
		return result;
	}

	bool card(const std::function<void()>& body, bool border)
	{
		const auto& theme = ctx_.current_theme;
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, theme.get_float4(theme_color::background_tertiary));
		int flags = ImGuiTableFlags_RowBg;
		ImVec2 size;
		if (border)
		{
			ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImGui::GetStyleColorVec4(ImGuiCol_Border));
			flags |= ImGuiTableFlags_BordersOuter;
			size = { ImGui::GetContentRegionAvail().x - table_border_size(), 0};
		}
		auto result = ImGui::BeginTable("##Card", 1, flags, size);
		if (result)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (body != nullptr)
			{
				body();
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleColor(1 + border);
		return result;
	}

	bool begin_main_menu(const std::string& label, bool enabled)
	{
		const auto& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{ 0.1765f, 0.1765f, 0.1765f, 1.f });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{ 0.1608f, 0.1608f, 0.1608f, 1.f });

		bool result = begin_menu(label, enabled);
		ImGui::PopStyleColor(3);
		return result;
	}

	bool begin_menu(const std::string& label, bool enabled)
	{
		return ImGui::BeginMenu(label.c_str(), enabled);
	}

	void end_menu()
	{
		ImGui::EndMenu();
	}

	float table_border_size()
	{
		static constexpr float table_border_size = 1.f; //FIXME: This is currently hardcoded in ImGui, change this when ImGui uses different border size
		return table_border_size;
	}

	float toggle_height()
	{
		return ImGui::GetFrameHeightWithSpacing() * 0.85f;
	}

	ImVec2 bigger_frame_size()
	{
		static constexpr float frame_padding_multiplier = 1.75f;
		const auto& style = ImGui::GetStyle();
		return style.FramePadding * frame_padding_multiplier;
	}

	bool is_item_disabled()
	{
		ImGuiContext& g = *GImGui;
		return (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
	}
}
