#include "toolbar_tool_popup.hpp"
#include <ui/widgets/common.hpp>
#include <ui/widgets/widget_list.hpp>
#include <ui/widgets/button_bar.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	std::ostream& operator<<(std::ostream& os, const toolbar_popup_entry& entry)
	{
		os << entry.display_name;
		return os;
	}

	toolbar_tool_popup::toolbar_tool_popup() : popup
	{
		"toolbar-popup",
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
	}, active_entry_{}, popup_entries_{ "##ToolbarPopupEntries", {} }
	{}

	void toolbar_tool_popup::set_active_entry(toolbar_group_entry* entry)
	{
		active_entry_ = entry;
	}

	void toolbar_tool_popup::set_position(ImVec2 pos)
	{
		pos_ = pos;
	}

	ImVec2 toolbar_tool_popup::position() const
	{
		return pos_;
	}

	void toolbar_tool_popup::pre_style()
	{
		ui::begin_rounded_popup_style();
		ImGui::SetNextWindowPos(pos_, ImGuiCond_Always);
	}

	void toolbar_tool_popup::post_style()
	{
		ui::end_rounded_popup_style();
	}

	void toolbar_tool_popup::on_display()
	{
		if (active_entry_ == nullptr) return;

		auto& tools = active_entry_->tools();
		std::vector<toolbar_popup_entry> popup_entries;
		popup_entries.reserve(tools.size());

		size_t selected_idx{};
		size_t i{};
		for (auto& tool : tools)
		{
			if (active_entry_->active_tool() == tool.get())
			{
				selected_idx = i;
			}
			popup_entries.push_back({ tool->display_name(), tool.get() });
			++i;
		}
		popup_entries_.set_items(popup_entries);
		popup_entries_.set_selected(selected_idx);
	}

	void toolbar_tool_popup::on_render()
	{
		render_tool_combo();
		render_body();
	}

	void toolbar_tool_popup::render_tool_combo()
	{
		if (active_entry_ == nullptr) return;

		ImGui::BeginDisabled(popup_entries_.item_count() <= 1);
		if (popup_entries_.render_with_label("Context"))
		{
			const auto& selected_item = popup_entries_.selected_item();
			active_entry_->set_active_tool(*selected_item.tool);
		}
		ImGui::EndDisabled();
	}

	void toolbar_tool_popup::render_body()
	{
		if (active_entry_ == nullptr) return;

		auto* active_tool = active_entry_->active_tool();
		if (active_tool == nullptr) return;

		ui::widget_list list;
		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("done") },
		};
		ui::button_bar<int> button_bar(buttons);
		if (active_tool->has_body())
		{
			ImGui::Separator();
			active_tool->render_popup_body(list, button_bar);
		}
		button_bar.set_default_button(0);
		list.add_raw([&button_bar, active_tool, this]()
		{
			ui::vertical_item_spacer();
			button_bar.render(0.f, true, [this, active_tool](const int& id)
			{
				if (id == 0)
				{
					active_tool->on_done();
					close();
				}
				else
				{
					active_tool->on_button_click(id);
				}
			});
			return true;
		});
		list.render();
	}
}
