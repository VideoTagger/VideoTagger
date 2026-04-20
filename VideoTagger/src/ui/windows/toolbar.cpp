#include "toolbar.hpp"
#include <ui/icons.hpp>

#include <core/app_context.hpp>
#include <events/toolbar/toolbar_tool_changed_event.hpp>
#include <events/toolbar/toolbar_register_tool_event.hpp>
#include <events/toolbar/toolbar_unregister_tool_event.hpp>

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

		register_listeners();
		add_default_tools();
	}

	void toolbar::add_tool(const toolbar_tool& tool)
	{
		auto tool_ptr = std::make_unique<toolbar_tool>(tool);
		auto ptr = tool_ptr.get();
		tools_.push_back(std::move(tool_ptr));

		ctx_.dispatch_event<toolbar_register_tool_event>(get_event_source(), *ptr);
	}

	void toolbar::remove_tool(const std::string& tool_id)
	{
		auto it = std::find_if(tools_.begin(), tools_.end(), [&tool_id](const std::unique_ptr<toolbar_tool>& tool)
		{
			return tool->id == tool_id;
		});

		if (it != tools_.end())
		{
			ctx_.dispatch_event<toolbar_unregister_tool_event>(get_event_source(), *it->get());
			tools_.erase(it);
		}
	}

	void toolbar::clear_tools()
	{
		for (auto& tool : tools_)
		{
			ctx_.dispatch_event<toolbar_unregister_tool_event>(get_event_source(), *tool);
		}
		tools_.clear();
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
		const auto source = get_event_source();

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
			bool is_selected = (active_tool_ == tool->id);
			if (ui::icon_toggle_button(tool->icon, is_selected) and !is_selected)
			{
				ctx_.dispatch_event<toolbar_tool_changed_event>(source, *tool);
			}
			ui::tooltip(tool->tooltip);
		}
	}

	void toolbar::add_default_tools()
	{
		toolbar_tool arrow_tool
		{
			"select",
			icons::tool_arrow,
			"Select"
		};
		add_tool(arrow_tool);
	}

	void toolbar::register_listeners()
	{
		auto source = get_event_source();
		ctx_.add_event_listener<toolbar_register_tool_event>([this, source](const toolbar_register_tool_event& event)
		{
			if (active_tool_.empty())
			{
				ctx_.dispatch_event<toolbar_tool_changed_event>(source, event.tool());
			}
		});

		ctx_.add_event_listener<toolbar_unregister_tool_event>([this, source](const toolbar_unregister_tool_event& event)
		{
			if (active_tool_ == event.tool().id)
			{
				ctx_.dispatch_event<toolbar_tool_changed_event>(source, toolbar_tool{});
			}
		});

		ctx_.add_event_listener<toolbar_tool_changed_event>([this](const toolbar_tool_changed_event& event)
		{
			auto new_id = event.tool().id;
			if (active_tool_ == new_id) return;

			if (new_id.empty())
			{
				active_tool_.clear();
				debug::log("Toolbar: Active tool cleared");
			}
			else
			{
				debug::log("Toolbar: Active tool changed to '{}'", new_id);
				active_tool_ = new_id;
			}
		});
	}
}
