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

		auto& toolbar = data();
		auto* active_entry = toolbar.active_entry();
		is_player_visible &= active_entry != nullptr;
		if (is_player_visible)
		{
			auto* active_tool = active_entry->active_tool();
			is_player_visible &= active_tool != nullptr;
			if (is_player_visible)
			{
				is_player_visible &= active_tool->property_column_count() > 0;
			}
		}

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
		//active_entry and active_tool shouldn't be null since the window is hidden if they are (checked in pre_style function)
		auto* active_entry = toolbar.active_entry();
		auto* active_tool = active_entry->active_tool();
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
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ style.ItemSpacing.x, 0.f});
		if (ImGui::BeginTable("##ToolProperties", static_cast<int>(active_tool->property_column_count()), ImGuiTableFlags_BordersInnerV, ImVec2{ 0.f, ImGui::GetFrameHeight() })) //ImGuiTableFlags_SizingFixedFit //ImGuiTableFlags_Resizable
		{
			ImGui::TableNextRow();
			active_tool->render_properties();
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
	}

	toolbar_session_data& tool_properties::data()
	{
		return ctx_.session.toolbar;
	}
}
