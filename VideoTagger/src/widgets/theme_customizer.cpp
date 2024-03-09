#include "theme_customizer.hpp"
#include <string>

#include <imgui_internal.h>
#include "icons.hpp"

namespace vt::widgets
{
	static bool draw_collapsing_header(const char* label)
	{
		auto& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{});
		std::string node_id = "##Node" + std::string(label);
		auto cx = ImGui::GetCursorPosX();
		bool result = ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		ImGui::PopStyleColor();
		auto icon = result ? icons::expand_less : icons::expand_more;

		ImGui::SameLine();
		auto px = ImGui::GetCursorPosX();
		ImGui::SetCursorPosX(px - (px - cx) + style.ItemInnerSpacing.x);
		//ImGui::SameLine(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::Text(label);
		ImGui::SameLine(ImGui::GetContentRegionMax().x - style.FramePadding.x - ImGui::CalcTextSize(icon).x);
		ImGui::Text(icon);
		return result;
	}

	theme_customizer::theme_customizer() : original_style{}, temp_style{ ImGui::GetStyle() }, is_open{ true }, live_preview{ true }
	{
		
	}

	void theme_customizer::render()
	{
		if (!is_open) return;

		auto& ref = ImGui::GetStyle();
		static bool output_only_modified = false;
		auto color_flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs;
		auto color_preview_flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder;
		auto table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH;

		if (ImGui::Begin("Theme Customizer", &is_open))
		{
			if (ImGui::Button("Export"))
			{

			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Live Preview", &live_preview))
			{
				if (live_preview)
				{
					original_style = ref;
				}
				else
				{
					ref = original_style;
				}
			}
			if (ImGui::BeginChild("##ThemeScrollableView"))
			{
				static auto draw_option = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
				{
					std::string color_label = "##" + label + info;
					auto& color = temp_style.Colors[col_id];
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
					ImGui::SameLine(); ImGui::Text(label.c_str());
					if (!info.empty())
					{
						info = "(" + info + ")";
						ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
					}
					ImGui::TableNextColumn();
				};

				if (draw_collapsing_header("Windows, Frames and Popups"))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, temp_style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Window", "Background", ImGuiCol_WindowBg);
						draw_option("Child", "Background", ImGuiCol_ChildBg);
						draw_option("Popup", "Background", ImGuiCol_PopupBg);
						draw_option("Border", "", ImGuiCol_Border);
						//draw_option("Border Shadow", "", ImGuiCol_BorderShadow);
						draw_option("Menu Bar", "Background", ImGuiCol_MenuBarBg);
						draw_option("Frame Background", "", ImGuiCol_FrameBg);
						draw_option("Frame Background", "Hovered", ImGuiCol_FrameBgHovered);
						draw_option("Frame Background", "Active", ImGuiCol_FrameBgActive);
						draw_option("Title Background", "", ImGuiCol_TitleBg);
						draw_option("Title Background", "Active", ImGuiCol_TitleBgActive);
						draw_option("Title Background", "Collapsed", ImGuiCol_TitleBgCollapsed);
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}
				if (draw_collapsing_header("Text"))
				{
					static auto draw_text = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::TextColored(color, "Text");
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, ref.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_text("Text", "", ImGuiCol_Text);
						draw_text("Text", "Disabled", ImGuiCol_TextDisabled);
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}
				if (draw_collapsing_header("Buttons"))
				{
					static auto draw_button = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::PushStyleColor(ImGuiCol_Button, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::Button("Button");
						ImGui::PopStyleColor(3);
					};

					static auto draw_checkbox = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::PushStyleColor(ImGuiCol_CheckMark, color);
						ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ref.Colors[ImGuiCol_FrameBg]);
						ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ref.Colors[ImGuiCol_FrameBg]);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						std::string check_label = color_label + "Checkbox";
						bool value = true;
						ImGui::TableNextColumn(); ImGui::Checkbox(check_label.c_str(), &value);
						ImGui::PopStyleColor(3);
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, ref.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_button("Button", "", ImGuiCol_Button);
						draw_button("Button", "Hovered", ImGuiCol_ButtonHovered);
						draw_button("Button", "Active", ImGuiCol_ButtonActive);
						draw_checkbox("Checkmark", "", ImGuiCol_CheckMark);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (draw_collapsing_header("Tabs"))
				{
					static auto draw_tab = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::PushStyleColor(ImGuiCol_Tab, color);
						ImGui::PushStyleColor(ImGuiCol_TabHovered, color);
						ImGui::PushStyleColor(ImGuiCol_TabActive, color);
						ImGui::PushStyleColor(ImGuiCol_TabUnfocused, color);
						ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, color);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						ImGui::TableNextColumn();
						std::string tab_bar_label = color_label + "TabBar";
						std::string tab_id = "Tab##" + color_label;
						if (ImGui::BeginTabBar(tab_bar_label.c_str()))
						{
							if (ImGui::BeginTabItem(tab_id.c_str())) ImGui::EndTabItem();
							ImGui::PopStyleColor(5);
							ImGui::EndTabBar();
						}
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, ref.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_tab("Tab", "", ImGuiCol_Tab);
						draw_tab("Tab", "Hovered", ImGuiCol_TabHovered);
						draw_tab("Tab", "Active", ImGuiCol_TabActive);
						draw_tab("Tab Unfocused", "", ImGuiCol_TabUnfocused);
						draw_tab("Tab Unfocused", "Active", ImGuiCol_TabUnfocusedActive);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (draw_collapsing_header("Scrollbars, Headers and Separators"))
				{
					static auto draw_header = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::PushStyleColor(ImGuiCol_Header, color);
						ImGui::PushStyleColor(ImGuiCol_HeaderHovered, color);
						ImGui::PushStyleColor(ImGuiCol_HeaderActive, color);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						std::string header_label = color_label + "Header";
						ImGui::SetNextItemOpen(false);
						ImGui::TableNextColumn(); ImGui::CollapsingHeader(header_label.c_str(), nullptr);
						ImGui::PopStyleColor(3);
					};

					static auto draw_separator = [&](const std::string& label, std::string info, ImGuiCol_ col_id)
					{
						std::string color_label = "##" + label + info;
						auto& color = temp_style.Colors[col_id];
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags);
						ImGui::PushStyleColor(ImGuiCol_Separator, color);
						ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, color);
						ImGui::PushStyleColor(ImGuiCol_SeparatorActive, color);
						ImGui::SameLine(); ImGui::Text(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled(info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 5.0f);
						ImGui::PopStyleColor(3);
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, ref.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Scrollbar", "Background", ImGuiCol_ScrollbarBg);
						draw_option("Scrollbar Grab", "", ImGuiCol_ScrollbarGrab);
						draw_option("Scrollbar Grab", "Hovered", ImGuiCol_ScrollbarGrabHovered);
						draw_option("Scrollbar Grab", "Active", ImGuiCol_ScrollbarGrabActive);

						draw_header("Header", "", ImGuiCol_Header);
						draw_header("Header", "Hovered", ImGuiCol_HeaderHovered);
						draw_header("Header", "Active", ImGuiCol_HeaderActive);

						draw_separator("Separator", "", ImGuiCol_Separator);
						draw_separator("Separator", "Hovered", ImGuiCol_SeparatorHovered);
						draw_separator("Separator", "Active", ImGuiCol_SeparatorActive);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();

		if (live_preview)
		{
			ref = temp_style;
		}
	}
}
