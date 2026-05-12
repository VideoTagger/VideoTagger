#include "tool_properties.hpp"
#include <ui/icons.hpp>
#include <core/app_context.hpp>
#include <widgets/video_player.hpp>
#include <ui/widgets/common.hpp>
#include <events/system/window/system_window_resize_event.hpp>

namespace vt::ui::windows
{
	tool_properties::tool_properties() : window
	{
		"Tool Properties", "tool-properties", "Tool Properties",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove
	}, reset_pos_{}
	{
		set_icon(icons::property);
		set_persistent(false);

		ctx_.add_event_listener<system_window_resize_event>([this](const system_window_resize_event& event)
		{
			reset_pos_ = true;
		});
	}

	void tool_properties::pre_style()
	{
		const auto& player = ctx_.get_window<widgets::video_player>();
		bool is_player_visible = player.is_visible() and ctx_.session.is_any_video_group_active();
		set_hidden(!is_player_visible);
		if (!is_player_visible)
		{
			return;
		}

		const auto& style = ImGui::GetStyle();

		ImGuiWindowClass win_class;
		win_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoUndocking;
		ImGui::SetNextWindowClass(&win_class);

		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.WindowPadding / 2.f);
		ui::begin_rounded_window_style();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ctx_.current_theme.get_float4(theme_color::background_secondary));

		auto player_rect = player.inner_rect();
		if (player_rect.has_value())
		{
			auto win_pos = player_rect->Min;
			win_pos += style.WindowPadding;
			ImGui::SetNextWindowPos(win_pos, reset_pos_ ? ImGuiCond_Always : ImGuiCond_Appearing);
			if (reset_pos_)
			{
				ctx_.tasks.run([this]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					reset_pos_ = false;
				});
			}
		}
	}

	void tool_properties::post_style()
	{
		if (!is_hidden())
		{
			ImGui::PopStyleColor();
			ui::end_rounded_window_style();
			ImGui::PopStyleVar();
		}
	}

	void tool_properties::on_render()
	{
		const auto& style = ImGui::GetStyle();

		auto& toolbar = data();
		const auto source = get_event_source();

		auto win_pos = ImGui::GetWindowPos();

		ui::drag_handle();
		if (ImGui::IsItemActive())
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				const auto& io = ImGui::GetIO();
				win_pos += io.MouseDelta;
				ImGui::SetWindowPos(win_pos);
			}
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}

		ImGui::SameLine(0.f, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
		if (ImGui::BeginTable("##ToolProperties", 2, ImGuiTableFlags_SizingFixedFit, ImVec2{ 0.f, ImGui::GetFrameHeight() }))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ui::icon_button(icons::settings);
			ImGui::TableNextColumn();
			ui::icon_button(icons::property);
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
	}

	toolbar_session_data& tool_properties::data()
	{
		return ctx_.session.toolbar;
	}
}
