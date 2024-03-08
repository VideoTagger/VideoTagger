#include "settings.hpp"
#include <imgui.h>

namespace vt::widgets
{
	bool settings(bool* open)
	{
		bool result = ImGui::Begin("Settings", open);
		if (result)
		{
			auto& io = ImGui::GetIO();
			auto& style = ImGui::GetStyle();

			if (ImGui::BeginTable("##SettingsColumns", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn(nullptr, 0, 0.25f);
				ImGui::TableSetupColumn(nullptr, 0, 0.75f);

				ImGui::TableNextColumn();
				auto avail_size = ImGui::GetContentRegionAvail();
				auto text_size = ImGui::CalcTextSize("Tab 0").x;
				ImVec2 padding = { avail_size.x - text_size - 2 * style.ItemSpacing.x, 0 };
				ImGui::Dummy(padding);
				ImGui::SameLine();
				ImGui::Text("Tab 1");
				ImGui::Dummy(padding);
				ImGui::SameLine();
				ImGui::TextDisabled("Tab 2");
				ImGui::Dummy(padding);
				ImGui::SameLine();
				ImGui::TextDisabled("Tab 3");

				ImGui::TableNextColumn();
				if (ImGui::BeginChild("##SettingsPanel", ImGui::GetContentRegionAvail()))
				{
#ifdef _DEBUG
					ImGui::SeparatorText("Debug Only");
					ImGui::DragFloat("Font Scale", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
#endif
					ImGui::EndChild();
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
		return result;
	}
}
