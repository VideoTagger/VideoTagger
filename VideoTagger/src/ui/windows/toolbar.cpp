#include "toolbar.hpp"
#include <ui/icons.hpp>

#include <core/app_context.hpp>

namespace vt::ui::windows
{
	toolbar::toolbar() : window
	{
		"Toolbar", "toolbar", "Toolbar",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove
	}, active_tool_{}
	{
		set_icon(icons::tool_arrow);
		set_persistent(false);

		toolbar_tool arrow_tool
		{
			icons::tool_arrow,
			"Select",
			[]()
			{
				debug::log("Arrow tool clicked");
			}
		};
		add_tool(arrow_tool);
	}

	void toolbar::add_tool(const toolbar_tool& tool)
	{
		tools_.push_back(tool);
	}

	void toolbar::pre_style()
	{
		const auto& style = ImGui::GetStyle();
		
		ImGuiWindowClass win_class;
		win_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoUndocking;
		ImGui::SetNextWindowClass(&win_class);

		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ui::begin_rounded_window_style();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ctx_.current_theme.get_float4(theme_color::background_secondary));
	}

	void toolbar::post_style()
	{
		ImGui::PopStyleColor();
		ui::end_rounded_window_style();
	}

	void toolbar::on_render()
	{
		const auto& style = ImGui::GetStyle();
		auto draw_list = ImGui::GetWindowDrawList();
		auto win_pos = ImGui::GetWindowPos();
		auto grabber_height = ImGui::GetTextLineHeight();
		ImRect grabber_rect{ win_pos, win_pos + ImVec2{ ImGui::GetWindowWidth(), grabber_height }};
		grabber_rect.Min += ImVec2{ style.WindowPadding.x, style.WindowPadding.y };
		grabber_rect.Max.x -= style.WindowPadding.x;

		draw_list->AddRectFilled(grabber_rect.Min, grabber_rect.Max, ImGui::GetColorU32(ImGuiCol_Header), 3.f);
		ImGui::SetCursorScreenPos(grabber_rect.Min);

		ui::rounded_button("##ToolbarGrabber", ImVec2{ grabber_rect.Max.x - grabber_rect.Min.x, grabber_rect.Max.y - grabber_rect.Min.y });

		if (ImGui::IsItemActive())
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				const auto& io = ImGui::GetIO();
				win_pos += io.MouseDelta;
				ImGui::SetWindowPos(win_pos);
			}
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}
		
		for (size_t i = 0; i < tools_.size(); ++i)
		{
			const auto& tool = tools_[i];
			if (ui::icon_toggle_button(tool.icon, (active_tool_ == i)) and active_tool_ != i)
			{
				active_tool_ = i;
				if (tool.on_click == nullptr) return;
				tool.on_click();
			}
			ui::tooltip(tool.tooltip);
		}
	}
}
