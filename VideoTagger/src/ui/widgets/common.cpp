#include "pch.hpp"
#include "common.hpp"

#include <imgui_toggle.h>
#include <imgui_toggle_palette.h>

#include <widgets/controls.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	//TODO: Get this out of theme struct (not hard coded)
	static constexpr auto accent_color = ImVec4{ 0.2588f, 0.6f, 0.8784f, 1.f };
	static constexpr auto accent_color_hover = ImVec4{ 0.2f, 0.5098f, 0.7804f, 1.f };
	static constexpr auto accent_color_active = ImVec4{ 0.1608f, 0.4353f, 0.6863f, 1.f };

	static constexpr auto button_color = ImVec4{ 0.1882f, 0.1882f, 0.1882f, 1.f };
	static constexpr auto button_color_hover = ImVec4{ 0.2078f, 0.2078f, 0.2078f, 1.f };
	static constexpr auto button_color_active = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };

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

	float toggle_height()
	{
		return ImGui::GetFrameHeight() * 0.85f;
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

	bool accent_button(const std::string& label, const ImVec2& size)
	{
		auto btn_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
		auto btn_hov_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		auto btn_actv_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

		//TODO: Move these colors into theme class
		bool is_disabled = widgets::is_item_disabled();
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
}
