#include "pch.hpp"
#include "common.hpp"

#include <imgui_toggle.h>
#include <imgui_toggle_palette.h>

#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	//TODO: Move this to theme struct (not hard coded)
	static constexpr auto accent_color = ImVec4{ 0.2588f, 0.6f, 0.8784f, 1.f };
	static constexpr auto accent_color_hover = ImVec4{ 0.2f, 0.5098f, 0.7804f, 1.f };
	static constexpr auto accent_color_active = ImVec4{ 0.1608f, 0.4353f, 0.6863f, 1.f };

	static constexpr auto button_color = ImVec4{ 0.1882f, 0.1882f, 0.1882f, 1.f };
	static constexpr auto button_color_hover = ImVec4{ 0.2078f, 0.2078f, 0.2078f, 1.f };
	static constexpr auto button_color_active = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };

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
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.FramePadding * 1.75f);
		bool result = ImGui::Button(label.c_str(), size);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		ImGui::PopStyleVar(2);
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

	bool accent_button(const std::string& label, const ImVec2& size)
	{
		auto btn_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		auto btn_hov_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		auto btn_actv_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		//TODO: Move these colors into theme class
		bool is_disabled = ui::is_item_disabled();
		ImGui::PushStyleColor(ImGuiCol_Button, is_disabled ? button_color : accent_color); // push button color if diabled or accent color if enabled
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, accent_color_hover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, accent_color_active);

		auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		text_color.x = 1.f - text_color.x;
		text_color.y = 1.f - text_color.y;
		text_color.z = 1.f - text_color.z;
		if (!is_disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, text_color);
		}
		bool result = rounded_button(label, size);
		ImGui::PopStyleColor(3 + !is_disabled);
		return result;
	}

	bool button(const std::string& label, const ImVec2& size)
	{
		auto btn_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		auto btn_hov_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		auto btn_actv_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		//TODO: Move these colors into theme class
		ImGui::PushStyleColor(ImGuiCol_Button, button_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_color_hover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_color_active);

		bool result = rounded_button(label, size);
		ImGui::PopStyleColor(3);
		return result;
	}

	static ImGuiToggleConfig toggle_config()
	{
		float size_scale = 1.f;

		const ImVec4 color_default = accent_color;
		const ImVec4 color_hover = accent_color_hover;
		const ImVec4 color_dim = accent_color_active;

		const ImVec2 material_size(37 * size_scale, 16 * size_scale);
		const float material_inset = -2.5f * size_scale;

		auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		auto text_color_dim = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
		auto text_color_inverted = text_color;
		text_color_inverted.x = 1.f - text_color_inverted.x;
		text_color_inverted.y = 1.f - text_color_inverted.y;
		text_color_inverted.z = 1.f - text_color_inverted.z;

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
		static auto config = toggle_config();
		return ImGui::Toggle(label.c_str(), &value, config);
    }

	bool checkbox(const std::string& label, bool& value)
	{
		auto& style = ImGui::GetStyle();

		bool result{};
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
		result = ImGui::Checkbox(label.c_str(), &value);
		ImGui::PopStyleVar();
		return result;
	}

	bool collapsing_header(const std::string& label, bool hide_background)
	{
		auto& style = ImGui::GetStyle();
		if (hide_background) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		std::string node_id = "##Node" + std::string(label);
		auto cx = ImGui::GetCursorPosX();
		bool result = ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);

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

	float toggle_height()
	{
		return ImGui::GetFrameHeight() * 0.85f;
	}

	bool is_item_disabled()
	{
		ImGuiContext& g = *GImGui;
		return (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
	}
}
