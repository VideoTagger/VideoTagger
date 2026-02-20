#include "pch.hpp"
#include "theme_customizer.hpp"

#include <utils/filesystem.hpp>
#include <core/debug.hpp>
#include "controls.hpp"
#include <utils/json.hpp>
#include <utils/color.hpp>

#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>

namespace vt::widgets
{
	theme_customizer::theme_customizer() : live_preview{ true } {}

	void theme_customizer::render(bool& is_open)
	{
		if (!is_open) return;

		auto& style = ImGui::GetStyle();
		auto& ref = ctx_.current_theme;
		static bool output_only_modified = false;
		auto color_flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs;
		auto color_preview_flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder;
		auto table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH;

		if (ImGui::Begin("Theme Customizer", &is_open))
		{
			if (ImGui::IsWindowAppearing())
			{
				temp_theme = ref;
				original_theme = ref;
			}

			ImGui::BeginDisabled();
			if (ui::icon_button(icons::download))
			{
				
			}
			ImGui::EndDisabled();
			//ui::tooltip("Import");
			ImGui::SameLine();
			if (ui::icon_button(icons::upload))
			{
				utils::dialog_filters filters{ utils::dialog_filter{ "VideoTagger Theme", "vttheme"} };
				auto result = utils::filesystem::save_file({}, filters, "Theme");
				if (result)
				{
					debug::log("Saving theme as {}", result.path.u8string());
					temp_theme.save(result.path);
				}
			}
			ui::tooltip("Export");

			ImGui::SameLine();
			if (ui::toggle("Live Preview", live_preview))
			{
				if (live_preview)
				{
					original_theme = ref;
				}
				else
				{
					ref = original_theme;
					ref.apply();
				}
			}
			ImGui::SameLine();
			bool is_dark = temp_theme.is_dark();
			ui::toggle("Dark", is_dark);
			temp_theme.set_dark(is_dark);
			if (ImGui::BeginChild("##ThemeScrollableView"))
			{
				static auto draw_option = [&](const std::string& label, std::string info, theme_color col_id)
				{
					std::string color_label = "##" + label + info;
					auto color = temp_theme.get_float4(col_id);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					
					if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
					{
						temp_theme.set_color(col_id, color);
					}
					ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
					if (!info.empty())
					{
						info = "(" + info + ")";
						ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
					}
					ImGui::TableNextColumn();
				};

				if (ui::collapsing_header("Base Colors"))
				{
					static auto draw_accent = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						ImGui::TableNextColumn();
						ImGui::BeginDisabled();
						ImGui::PushStyleColor(ImGuiCol_Button, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
						ctx_.current_theme.push_color(theme_color::accent_light, color);
						ctx_.current_theme.push_color(theme_color::accent_medium, color);
						ctx_.current_theme.push_color(theme_color::accent_dark, color);
						ctx_.current_theme.push_color(theme_color::secondary_light, color);
						ctx_.current_theme.push_color(theme_color::secondary_medium, color);
						ctx_.current_theme.push_color(theme_color::secondary_dark, color);
						ImGui::PushStyleColor(ImGuiCol_TextDisabled, temp_theme.get_float4(theme_color::text_inverted));
						ui::accent_button("Button");
						ImGui::PopStyleColor(4);
						ImGui::EndDisabled();
						ImGui::SameLine();
						bool value = true;
						
						ImGui::BeginDisabled();
						ui::toggle("", value);
						ImGui::EndDisabled();
						ctx_.current_theme.pop_color(6);
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##BaseColors", 2, table_flags))
					{
						draw_accent("Accent", "Light", theme_color::accent_light);
						draw_accent("Accent", "Medium", theme_color::accent_medium);
						draw_accent("Accent", "Dark", theme_color::accent_dark);
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (ui::collapsing_header("Windows, Frames and Popups"))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Window", "Background", theme_color::window_background);
						draw_option("Child", "Background", theme_color::child_background);
						draw_option("Popup", "Background", theme_color::popup_background);
						draw_option("Border", "", theme_color::border);
						//draw_option("Border Shadow", "", ImGuiCol_BorderShadow);
						draw_option("Menu Bar", "Background", theme_color::menubar_background);
						draw_option("Frame Background", "", theme_color::frame_background_normal);
						draw_option("Frame Background", "Hovered", theme_color::frame_background_hover);
						draw_option("Frame Background", "Active", theme_color::frame_background_active);
						draw_option("Title Background", "", theme_color::title_background_normal);
						draw_option("Title Background", "Active", theme_color::title_background_active);
						draw_option("Title Background", "Collapsed", theme_color::title_background_collapsed);
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}
				if (ui::collapsing_header("Text"))
				{
					static auto draw_text = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::TextColored(color, "Text");
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_text("Text", "", theme_color::text_normal);
						draw_text("Text", "Inverted", theme_color::text_inverted);
						draw_text("Text", "Disabled", theme_color::text_disabled);
						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}
				if (ui::collapsing_header("Buttons"))
				{
					static auto draw_button = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::PushStyleColor(ImGuiCol_Button, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::Button("Button");
						ImGui::PopStyleColor(3);
					};

					static auto draw_checkbox = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::PushStyleColor(ImGuiCol_CheckMark, color);
						ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, temp_theme.get_float4(theme_color::frame_background_normal));
						ImGui::PushStyleColor(ImGuiCol_FrameBgActive, temp_theme.get_float4(theme_color::frame_background_normal));
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						std::string check_label = color_label + "Checkbox";
						bool value = true;
						ImGui::TableNextColumn(); ImGui::Checkbox(check_label.c_str(), &value);
						ImGui::PopStyleColor(3);
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_button("Button", "", theme_color::button_normal);
						draw_button("Button", "Hovered", theme_color::button_hover);
						draw_button("Button", "Active", theme_color::button_active);
						draw_checkbox("Checkmark", "", theme_color::checkmark);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (ui::collapsing_header("Tabs"))
				{
					static auto draw_tab = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::PushStyleColor(ImGuiCol_Tab, color);
						ImGui::PushStyleColor(ImGuiCol_TabHovered, color);
						ImGui::PushStyleColor(ImGuiCol_TabActive, color);
						ImGui::PushStyleColor(ImGuiCol_TabUnfocused, color);
						ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, color);
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
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

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_tab("Tab Focused", "", theme_color::tab_focused_normal);
						draw_tab("Tab Focused", "Hovered", theme_color::tab_focused_hover);
						draw_tab("Tab Focused", "Active", theme_color::tab_focused_active);
						draw_tab("Tab Unfocused", "", theme_color::tab_unfocused_normal);
						draw_tab("Tab Unfocused", "Active", theme_color::tab_unfocused_active);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (ui::collapsing_header("Scrollbars, Headers and Separators"))
				{
					static auto draw_header = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::PushStyleColor(ImGuiCol_Header, color);
						ImGui::PushStyleColor(ImGuiCol_HeaderHovered, color);
						ImGui::PushStyleColor(ImGuiCol_HeaderActive, color);
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						std::string header_label = color_label + "Header";
						ImGui::SetNextItemOpen(false);
						ImGui::TableNextColumn(); ImGui::CollapsingHeader(header_label.c_str(), nullptr);
						ImGui::PopStyleColor(3);
					};

					static auto draw_separator = [&](const std::string& label, std::string info, theme_color col_id)
					{
						std::string color_label = "##" + label + info;
						auto color = temp_theme.get_float4(col_id);
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						if (ImGui::ColorEdit4(color_label.c_str(), (float*)&color, color_flags))
						{
							temp_theme.set_color(col_id, color);
						}
						ImGui::PushStyleColor(ImGuiCol_Separator, color);
						ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, color);
						ImGui::PushStyleColor(ImGuiCol_SeparatorActive, color);
						ImGui::SameLine(); ImGui::TextUnformatted(label.c_str());
						if (!info.empty())
						{
							info = "(" + info + ")";
							ImGui::SameLine(); ImGui::TextDisabled("%s", info.c_str());
						}
						ImGui::TableNextColumn(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 5.0f);
						ImGui::PopStyleColor(3);
					};

					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Scrollbar", "Background", theme_color::scrollbar_background);
						draw_option("Scrollbar Grab", "", theme_color::scrollbar_grab_normal);
						draw_option("Scrollbar Grab", "Hovered", theme_color::scrollbar_grab_hover);
						draw_option("Scrollbar Grab", "Active", theme_color::scrollbar_grab_active);

						draw_header("Header", "", theme_color::header_normal);
						draw_header("Header", "Hovered", theme_color::header_hover);
						draw_header("Header", "Active", theme_color::header_active);

						draw_separator("Separator", "", theme_color::separator_normal);
						draw_separator("Separator", "Hovered", theme_color::separator_hover);
						draw_separator("Separator", "Active", theme_color::separator_active);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (ui::collapsing_header("Viewport"))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Axis", "X", theme_color::axis_x);
						draw_option("Axis", "Y", theme_color::axis_y);
						draw_option("Axis", "Z", theme_color::axis_z);

						ImGui::EndTable();
					}
					ImGui::PopStyleColor();
				}

				if (ui::collapsing_header("Console"))
				{
					ImGui::PushStyleColor(ImGuiCol_TableRowBg, style.Colors[ImGuiCol_MenuBarBg]);
					if (ImGui::BeginTable("##Background", 2, table_flags))
					{
						draw_option("Console Log", "Info", theme_color::console_info);
						ImGui::TextColored(temp_theme.get_float4(theme_color::console_info), "%s", icons::info);
						draw_option("Console Log", "Warn", theme_color::console_warn);
						ImGui::TextColored(temp_theme.get_float4(theme_color::console_warn), "%s", icons::warning);
						draw_option("Console Log", "Error", theme_color::console_error);
						ImGui::TextColored(temp_theme.get_float4(theme_color::console_error), "%s", icons::error);

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
			temp_theme.apply();
			ref = temp_theme;
			ref.apply();
		}
	}
}
