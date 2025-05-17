#include "pch.hpp"
#include "options_popup.hpp"

#include <widgets/controls.hpp>
#include <core/app_context.hpp>

namespace vt::ui
{
	options_popup::options_popup(std::optional<bool*> open) : modal_popup{ "Options", open, ImGuiWindowFlags_NoTitleBar } {}

	void options_popup::on_display()
	{

	}

	void options_popup::on_render()
	{
		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		ImGui::Separator();

		bool is_popup_over{};
		if (ImGui::BeginTable("##OptionsColumns", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableSetupColumn(nullptr, 0, 0.25f);
			ImGui::TableSetupColumn(nullptr, 0, 0.75f);

			ImGui::TableNextColumn();
			size_t group_id = 0;
			size_t group_count = groups_.size();

			if (ImGui::BeginChild("##OptionsTabPanel", ImGui::GetContentRegionAvail()))
			{
				for (auto& [group, tabs] : groups_)
				{
					if (widgets::collapsing_header(group.c_str(), true))
					{
						for (auto& [name, body] : tabs)
						{
							bool inactive = name != active_tab_ or group != active_group_;

							if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
							if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf) and ImGui::IsItemClicked())
							{
								active_group_ = group;
								active_tab_ = name;
							}
							if (inactive) ImGui::PopStyleColor();
						}

						if (++group_id != group_count)
						{
							ImGui::Dummy({ 0, 0.25f * ImGui::GetTextLineHeightWithSpacing() });
						}
					}
				}
				ImGui::EndChild();
			}
			//ImGui::PopStyleVar();

			ImGui::TableNextColumn();
			if (ImGui::BeginChild("##OptionsBodyPanel", ImGui::GetContentRegionAvail()))
			{
				auto it = groups_.find(active_group_);
				if (it != groups_.end())
				{
					const auto& tabs = it->second;
					auto tab = tabs.find(active_tab_);
					if (tab != tabs.end())
					{
						std::invoke(tab->second);
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndTable();
		}
	}

	bool options_popup::pre_render()
	{
		ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Always);
		return modal_popup::pre_render();
	}

	void options_popup::set_active_tab(const std::string& group, const std::string& name)
	{
		active_group_ = group;
		active_tab_ = name;
	}

	std::function<void()>& options_popup::operator()(const std::string& group, const std::string& name)
	{
		return groups_[group][name];
	}
}
