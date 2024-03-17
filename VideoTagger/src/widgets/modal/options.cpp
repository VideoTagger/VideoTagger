#define IMGUI_DEFINE_MATH_OPERATORS
#include "options.hpp"
#include <imgui.h>
#include <widgets/controls.hpp>
#include <widgets/icons.hpp>
#include <core/app_context.hpp>

namespace vt::widgets::modal
{
	options::options() : active_tab{}
	{
	}

	bool options::render(bool* open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
		auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		bool result = ImGui::BeginPopupModal("Options", open, flags);
		if (result)
		{
			auto& io = ImGui::GetIO();
			auto& style = ImGui::GetStyle();

			auto icon = icons::exit;
			ImGui::PushFont(ctx_.fonts["title"]);
			ImGui::Text("Options");
			ImGui::PopFont();
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(icon).x - style.WindowPadding.x - style.WindowRounding);
			if (icon_button(icon) and open != nullptr)
			{
				*open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();
			
			if (ImGui::BeginTable("##OptionsColumns", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp, ImGui::GetContentRegionAvail()))
			{
				ImGui::TableSetupColumn(nullptr, 0, 0.25f);
				ImGui::TableSetupColumn(nullptr, 0, 0.75f);

				ImGui::TableNextColumn();
				auto avail_size = ImGui::GetContentRegionAvail();
				auto text_size = ImGui::CalcTextSize("Tab 0").x;
				ImVec2 padding = { avail_size.x - text_size - 2 * style.ItemSpacing.x, 0 };
				//ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.125f, 0.0f));
				size_t group_id = 0;
				size_t group_count = groups.size();

				if (ImGui::BeginChild("##OptionsTabPanel", ImGui::GetContentRegionAvail()))
				{
					for (auto& [group, tabs] : groups)
					{
						if (collapsing_header(group.c_str(), true))
						{
							for (auto& [name, body] : tabs)
							{
								bool inactive = name != active_tab or group != active_group;

								if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
								if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf) and ImGui::IsItemClicked())
								{
									active_group = group;
									active_tab = name;
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
					auto it = groups.find(active_group);
					if (it != groups.end())
					{
						const auto& tabs = it->second;
						auto tab = tabs.find(active_tab);
						if (tab != tabs.end())
						{
							std::invoke(tab->second);
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndTable();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape) and open != nullptr)
			{
				*open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		return result;
	}

	void options::set_active_tab(const std::string& group, const std::string& name)
	{
		active_group = group;
		active_tab = name;
	}

	std::function<void()>& options::operator()(const std::string& group, const std::string& name)
	{
		return groups[group][name];
	}
}
