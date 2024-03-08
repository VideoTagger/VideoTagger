#include "theme_customizer.hpp"
#include <imgui_internal.h>

namespace vt::widgets
{
	theme_customizer::theme_customizer() : style{ ImGui::GetStyle() }, is_open{ true }
	{

	}

	void theme_customizer::render()
	{
		if (!is_open) return;

		auto& ref = ImGui::GetStyle();
		static int output_dest = 0;
		static bool output_only_modified = false;

		if (ImGui::Begin("Theme Customizer", &is_open))
		{
			if (ImGui::Button("Export"))
			{
				if (output_dest == 0)
					ImGui::LogToClipboard();
				else
					ImGui::LogToTTY();
				ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const ImVec4& col = style.Colors[i];
					const char* name = ImGui::GetStyleColorName(i);
					if (!output_only_modified || memcmp(&col, &ref.Colors[i], sizeof(ImVec4)) != 0)
						ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
							name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
				}
				ImGui::LogFinish();
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine();
			if (ImGui::Button("Dark Theme"))
			{
				ImGui::StyleColorsDark();
				style = ref;
			}
			ImGui::SameLine();
			if (ImGui::Button("Light Theme"))
			{
				ImGui::StyleColorsLight();
				style = ref;
			}
			//ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
			/*/
			if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
			if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
			if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
			*/

			ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10), ImVec2(FLT_MAX, FLT_MAX));
			ImGui::BeginChild("##colors", ImVec2(0, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHSV | alpha_flags);
				if (memcmp(&style.Colors[i], &ref.Colors[i], sizeof(ImVec4)) != 0)
				{
					// Tips: in a real user application, you may want to merge and use an icon font into the main font,
					// so instead of "Save"/"Revert" you'd use icons!
					// Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref.Colors[i] = style.Colors[i]; }
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref.Colors[i]; }
				}
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();
		}
		ImGui::End();
	}
}
