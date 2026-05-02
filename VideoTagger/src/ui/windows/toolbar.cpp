#include "toolbar.hpp"
#include <ui/icons.hpp>

#include <core/app_context.hpp>
#include <events/toolbar/toolbar_tool_changed_event.hpp>
#include <widgets/video_player.hpp>
#include <events/system/window/system_window_resize_event.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>
#include <events/timeline/segment_selected_event.hpp>

namespace vt::ui::windows
{
	toolbar::toolbar() : window
	{
		"Toolbar", "toolbar", "Toolbar",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove
	}, reset_pos_{}
	{
		set_icon(icons::tool_arrow);
		set_persistent(false);

		ctx_.add_event_listener<system_window_resize_event>([this](const system_window_resize_event& event)
		{
			reset_pos_ = true;
		});
		ctx_.add_event_listener<toolbar_register_tool_event>([this](const toolbar_register_tool_event& event)
		{
			reset_pos_ = true;
		});
		ctx_.add_event_listener<toolbar_unregister_tool_event>([this](const toolbar_unregister_tool_event& event)
		{
			reset_pos_ = true;
		});

		ctx_.add_event_listener<segment_selected_event>([this](const segment_selected_event& event)
		{
			auto& tb_data = data();
			auto source = get_event_source();
			tb_data.remove_non_persistent(source);

			bool is_only_one_segment_selected = ctx_.session.is_one_segment_selected();
			if (is_only_one_segment_selected)
			{
				tb_data.request_register_tools(source);
			}
		});
	}

	void toolbar::pre_style()
	{
		const auto& player = ctx_.get_window<widgets::video_player>();
		bool is_player_visible = player.is_visible();
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

		auto player_rect = player.draw_rect();
		if (player_rect.has_value())
		{
			auto win_pos = player_rect->Min;
			win_pos.x += style.WindowPadding.x;
			win_pos.y = (player_rect->Min.y + player_rect->Max.y) / 2.f - calc_win_height() / 2.f;
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

	void toolbar::post_style()
	{
		if (!is_hidden())
		{
			ImGui::PopStyleColor();
			ui::end_rounded_window_style();
			ImGui::PopStyleVar();
		}
	}

	void toolbar::on_render()
	{
		const auto& toolbar = data();
		const auto source = get_event_source();
		auto grabber_height = ImGui::GetTextLineHeight();

		const auto& style = ImGui::GetStyle();
		auto draw_list = ImGui::GetWindowDrawList();
		auto win_pos = ImGui::GetWindowPos();
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
		
		const auto& tools = toolbar.tools();
		for (const auto& [tool_id, tool_instances] : tools)
		{
			for (const auto& tool : tool_instances)
			{
				bool is_selected = toolbar.is_tool_active(tool->id);
				if (ui::icon_toggle_button(tool->icon, is_selected) and !is_selected)
				{
					ctx_.dispatch_event<toolbar_tool_changed_event>(source, *tool);
				}
				ui::tooltip(tool->tooltip);
			}
		}
	}

	toolbar_session_data& toolbar::data()
	{
		return ctx_.session.toolbar;
	}

	float toolbar::calc_win_height()
	{
		const auto& style = ImGui::GetStyle();
		const auto& toolbar = data();
		const auto& tools = toolbar.tools();
		auto button_size = ImGui::GetFrameHeightWithSpacing();
		auto grabber_height = ImGui::GetFrameHeightWithSpacing();
		return grabber_height + button_size * tools.size() + style.WindowPadding.y; // style.WindowPadding.y * 2 / 2
	}
}
