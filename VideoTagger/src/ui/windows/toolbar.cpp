#include "toolbar.hpp"
#include <ui/icons.hpp>

#include <core/app_context.hpp>
#include <widgets/video_player.hpp>
#include <events/system/window/system_window_resize_event.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>
#include <events/timeline/segment_selected_event.hpp>
#include <events/timeline/segment_deselected_event.hpp>
#include <events/toolbar/toolbar_tool_change_request.hpp>

namespace vt::ui::windows
{
	toolbar::toolbar() : window
	{
		"Toolbar", "toolbar", "Toolbar",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove
	}, reset_pos_{}, tool_popup_{ new_popup<ui::toolbar_tool_popup>() }
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

		ctx_.add_event_listener<segment_deselected_event>([this](const segment_deselected_event& event)
		{
			auto& tb_data = data();
			auto source = get_event_source();
			tb_data.remove_non_persistent(source);
		});
	}

	void toolbar::pre_style()
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
		auto& toolbar = data();
		const auto source = get_event_source();
		auto grabber_height = ImGui::GetTextLineHeight();

		const auto& style = ImGui::GetStyle();
		auto win_pos = ImGui::GetWindowPos();
		ImRect grabber_rect{ win_pos, win_pos + ImVec2{ ImGui::GetWindowWidth(), grabber_height }};
		grabber_rect.Min += style.WindowPadding;
		grabber_rect.Max.x -= style.WindowPadding.x;

		ImGui::SetCursorScreenPos(grabber_rect.Min);

		ui::rounded_button("##ToolbarGrabber", grabber_rect.GetSize());

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
		
		bool render_separator = false;
		for (auto& [group_id, group] : toolbar.groups())
		{
			if (group.empty()) continue;

			if (render_separator)
			{
				render_separator = false;
				ImGui::Separator();
			}
			for (auto& pair : group.entries_sorted())
			{
				auto& [tool_id, entry] = pair;

				const auto& spec = entry->specification();
				bool is_selected = toolbar.is_tool_active(spec.id);
				bool always_show = entry->should_always_display_body();
				bool should_show_popup = always_show or entry->tool_count() > 1 or entry->has_any_tool_body();

				bool was_toggled = false;
				if (ui::icon_toggle_button(spec.icon, is_selected)) // and !is_selected
				{
					was_toggled = true;
				}

				bool should_be_selected = !is_selected and was_toggled;

				auto toggle_pos = ImGui::GetItemRectMin();
				ui::tooltip(spec.tooltip);
				render_separator = true;

				if ((is_selected or should_be_selected) and should_show_popup)
				{
					auto window = ImGui::GetCurrentWindow();
					auto window_rect = window->Rect();

					ImVec2 popup_pos{ window_rect.Max.x, toggle_pos.y };
					tool_popup_->set_position(popup_pos);
				}

				if (was_toggled)
				{
					if (should_show_popup)
					{
						tool_popup_->open();
					}
				}
				if (should_be_selected)
				{
					tool_popup_->set_active_entry(entry);
					ctx_.dispatch_event<toolbar_tool_change_request_event>(source, group, *entry, *entry->front());
				}
			}
		}
		tool_popup_->render();
	}

	toolbar_session_data& toolbar::data()
	{
		return ctx_.session.toolbar;
	}

	float toolbar::calc_win_height()
	{
		const auto& style = ImGui::GetStyle();
		const auto& toolbar = data();
		const auto& groups = toolbar.groups();
		auto button_size = ImGui::GetFrameHeightWithSpacing();
		auto grabber_height = ImGui::GetFrameHeightWithSpacing();
		auto separator_height = button_size;

		size_t tool_count = 0;
		size_t separator_count = 0;
		for (const auto& [group_id, group] : groups)
		{
			tool_count += group.size();
			if (!group.empty())
			{
				++separator_count;
			}
		}
		return grabber_height + button_size * tool_count + separator_count * separator_height + style.WindowPadding.y; // style.WindowPadding.y * 2 / 2
	}
}
