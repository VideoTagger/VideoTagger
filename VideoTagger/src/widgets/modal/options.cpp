#define IMGUI_DEFINE_MATH_OPERATORS
#include "options.hpp"
#include <imgui.h>
#include <widgets/buttons.hpp>
#include <widgets/icons.hpp>
#include <core/app_context.hpp>

namespace vt::widgets::modal
{
	bool options(bool* open)
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
			if (icon_button(icon))
			{
				*open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();
			
			if (ImGui::BeginTable("##OptionsColumns", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable, ImGui::GetContentRegionAvail()))
			{
				ImGui::TableSetupColumn(nullptr, 0, 0.25f);
				ImGui::TableSetupColumn(nullptr, 0, 0.75f);

				ImGui::TableNextColumn();
				auto avail_size = ImGui::GetContentRegionAvail();
				auto text_size = ImGui::CalcTextSize("Tab 0").x;
				ImVec2 padding = { avail_size.x - text_size - 2 * style.ItemSpacing.x, 0 };
				//TODO: Replace with actual tabs
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
				if (ImGui::BeginChild("##OptionsPanel", ImGui::GetContentRegionAvail()))
				{
#ifdef _DEBUG
					ImGui::SeparatorText("Debug Only");
					ImGui::DragFloat("Font Scale", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
#endif
					ImGui::EndChild();
				}
				ImGui::EndTable();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		return result;
	}
}
