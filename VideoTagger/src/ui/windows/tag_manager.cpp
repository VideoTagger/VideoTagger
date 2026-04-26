#include "pch.hpp"
#include "tag_manager.hpp"

#include <utils/random.hpp>
#include <widgets/controls.hpp>
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>
#include <utils/drag_drop.hpp>
#include <utils/string.hpp>
#include <ui/widgets/text_input.hpp>
#include <events/tags/tag_add_request_event.hpp>
#include <events/tags/tag_rename_request_event.hpp>
#include <events/tags/tag_delete_request_event.hpp>

static constexpr ImGuiColorEditFlags color_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip;

namespace vt::ui::windows
{
	//static void draw_tag_attribute(const std::string& name, tag_attribute& attr, const std::function<void(const std::string&)>& on_name_change, const std::function<void(tag_attribute::type)>& on_type_change, const std::function<void()>& on_delete)
	//{
	//	const auto& style = ImGui::GetStyle();

	//	bool selected{};
	//	bool row_hovered = widgets::table_hovered_row_style();

	//	ImGui::PushID(&attr);
	//	ImGui::TableNextColumn();
	//	ImGui::BeginGroup();
	//	widgets::frame_color_indicator(3.f, tag_attribute::type_color(attr.type_));
	//	ImGui::SameLine();
	//	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	//	std::string new_name = name;
	//	ui::text_input input("##TagAttributeName", new_name, "Attribute Name...");
	//	input.set_flags(ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
	//	if (input.render())
	//	{
	//		on_name_change(input.trimmed_input());
	//	}
	//	ImGui::TableNextColumn();

	//	int current_type = (int)attr.type_;
	//	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	//	if (ImGui::Combo("##TagAttributeType", &current_type, tag_attribute::types_str, (int)tag_attribute::type_count))
	//	{
	//		on_type_change((tag_attribute::type)current_type);
	//	}

	//	switch (attr.type_)
	//	{
	//	case tag_attribute::type::bool_: ui::tooltip("Value: True/False"); break;
	//	case tag_attribute::type::float_: ui::tooltip("Value: Float (64 bit)"); break;
	//	case tag_attribute::type::integer: ui::tooltip("Value: Integer (64 bit)"); break;
	//	case tag_attribute::type::string: ui::tooltip("Value: Text"); break;
	//	case tag_attribute::type::shape:
	//	{
	//		std::string shapes;
	//		size_t i{};
	//		for (auto type : shape::types)
	//		{
	//			shapes += utils::string::to_titlecase(shape::type_str(type));
	//			if (++i < shape::types.size())
	//			{
	//				shapes += "/";
	//			}
	//		}
	//		ui::tooltip(fmt::format("Value: {}", shapes).c_str());
	//	}
	//	break;
	//	}

	//	ImGui::EndGroup();
	//	if (ImGui::BeginPopupContextItem("##TagAttributeCtxMenu"))
	//	{
	//		std::string menu_name = fmt::format("{} Delete", icons::delete_);
	//		if (ImGui::MenuItem(menu_name.c_str()))
	//		{
	//			on_delete();
	//		}
	//		ImGui::EndPopup();
	//	}
	//	if (row_hovered and ImGui::IsMouseClicked(1))
	//	{
	//		ImGui::OpenPopup("##TagAttributeCtxMenu");
	//	}
	//	ImGui::PopID();
	//}

	tag_manager::tag_manager() :
		window{ "Tag Manager", "tag-manager", "Tag Manager", ImGuiWindowFlags_NoScrollbar }
	{
		set_icon(icons::tags);
	}

	void tag_manager::on_display()
	{
		filter_.clear();
		color_ref_ = ctx_.current_project->tags.end();
	}

	void tag_manager::on_render()
	{
		event_source event_source_ = get_event_source();
		auto& tags = ctx_.current_project->tags;
		//TODO: Maybe extract some stuff into separate functions for better readability

		bool return_value = false;
		auto& style = ImGui::GetStyle();

		bool update_all = false;
		bool update_state = false;


		{
			static constexpr float tag_column_width = 100;

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			if (ui::icon_button(icons::add_tag))
			{
				new_tag_popup_ = std::make_unique<new_tag_popup>(event_source_);
			}
			ui::tooltip("Add Tag");

			ImGui::SameLine();
			widgets::search_bar
			(
				"##VideoGroupBrowserSearch",
				ctx_.lang->get("search_hint").c_str(),
				filter_,
				ImGui::GetContentRegionAvail().x - 2 * (ImGui::CalcTextSize(icons::toggle_less).x + 2 * style.FramePadding.x)
			);

			ImGui::SameLine();
			if (ui::icon_button(icons::toggle_more))
			{
				update_state = true;
				update_all = true;
			}
			ui::tooltip("Expand All");

			ImGui::SameLine();
			ImGui::PopStyleVar();
			if (ui::icon_button(icons::toggle_less))
			{
				update_state = false;
				update_all = true;
			}
			ui::tooltip("Collapse All");

			ImGui::Separator();

			std::vector<std::string> tokens;
			if (!filter_.empty())
			{
				tokens = utils::string::split(utils::string::to_lowercase(utils::string::trim_whitespace(filter_)), ' ');
			}

			//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
			bool is_scrollable_list_open = ImGui::BeginChild("##ScrollableTagList", ImGui::GetContentRegionAvail());

			//ImGui::PopStyleVar();
			if (is_scrollable_list_open)
			{
				size_t filter_passes{};
				int id{};

				for (auto it = tags.begin(); it != tags.end();)
				{
					auto& tag = *it;

					bool passes_filter = true;
					for (const auto& token : tokens)
					{
						auto ttoken = utils::string::trim_whitespace(token);
						std::string name = utils::string::to_lowercase(tag.name);
						passes_filter &= name.find(ttoken) != std::string::npos;
					}

					if (!passes_filter)
					{
						++it;
						continue;
					}
					++filter_passes;


					//ImGui::TableNextColumn();
					ImGui::PushID(id++);
					/*
					if (icon_button(icons::close))
					{
						tags.erase(tag.name);
						ctx_.is_project_dirty = true;
						ImGui::PopStyleVar();
						ImGui::PopID();
						break;
					}
					*/
					auto color = ImGui::ColorConvertU32ToFloat4(tag.color);
					bool open_color_picker = false;

					if (update_all)
					{
						ImGui::SetNextItemOpen(update_state);
					}

					bool node_open = widgets::begin_collapsible("##TagManagerNode", tag.name, 0, icons::label, color, [&color, &tag]()
					{
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
						{
							utils::drag_drop::set_payload("Tag", tag.name.c_str());
							ImGui::TextColored(color, icons::label);
							ImGui::SameLine();
							ImGui::TextUnformatted(tag.name.c_str());
							ImGui::EndDragDropSource();
						}
					});

					if (ImGui::BeginPopupContextItem("##TagCtxMenu"))
					{
						std::string menu_name = fmt::format("{} Delete", icons::delete_);
						if (ImGui::MenuItem(menu_name.c_str()))
						{
							delete_tag_popup_ = std::make_unique<delete_tag_popup>(event_source_, it->name);
						}
						ImGui::EndPopup();
					}
					if (ImGui::IsItemHovered() and ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("##TagCtxMenu");
					}

					if (node_open)
					{
						ui::card([&]()
						{
							ImGui::TableNextColumn();
							ImGui::Columns(2, "##TagColumnSeparator");
							ImGui::Text("Name");
							ImGui::NextColumn();
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

							//TODO: Add filtering & read the tag with a new name since std::map is used as a container (why not std::vector??)

							tag_name_ = tag.name;
							ui::text_input input("##TagNameInput", tag_name_, "Tag Name...");
							input.set_flags(ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
							if (input.render())
							{
								rename_tag_popup_ = std::make_unique<rename_tag_popup>(event_source_, tag.name, tag_name_);
							}
							ImGui::NextColumn();
							ImGui::TextUnformatted("Color");
							ImGui::NextColumn();
							if (ui::color_button("##ColorButton", color, color_button_flags))
							{
								color_ref_ = it;
								open_color_picker = true;
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
							}
							ImGui::Columns();


							// Attributes
							{
								ImGui::Separator();
								const auto& theme = ctx_.current_theme;
								//ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImGui::GetStyleColorVec4(ImGuiCol_Border));
								ImGui::PushStyleColor(ImGuiCol_TableRowBg, theme.get_float4(theme_color::background_secondary));
								ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, theme.get_float4(theme_color::background_secondary));
								if (ImGui::BeginTable("##Attributes", 2, ImGuiTableFlags_BordersOuter, { ImGui::GetContentRegionAvail().x - ui::table_border_size(), 0 }))
								{
									ImGui::TableSetupColumn("Name");
									ImGui::TableSetupColumn("Type");

									ImGui::TableNextColumn();
									if (ui::icon_button(icons::add))
									{
										add_tag_attribute_popup_ = std::make_unique<add_tag_attribute_popup>(event_source_, tag_name_);
									}
									ImGui::SameLine();
									ImGui::AlignTextToFramePadding();
									ImGui::TextUnformatted("Attributes");

									if (!tag.attributes.empty())
									{
										ImGui::BeginDisabled();
										ImGui::TableHeadersRow();
										ImGui::EndDisabled();
									}
									std::string new_name_candidate;
									std::string new_name;
									for (auto it = tag.attributes.begin(); it != tag.attributes.end();)
									{
										//bool next = true;
										//auto& [name, attr] = *it;
										//ImGui::TableNextRow();
										//draw_tag_attribute(name, attr,
										//[&new_name_candidate, &new_name, &name](const std::string& nname)
										//{
										//	new_name_candidate = name;
										//	new_name = nname;
										//},
										//[&attr](const tag_attribute::type new_type)
										//{
										//	attr.type_ = new_type;
										//},
										//[&tag, &it, &name, &next]()
										//{
										//	it = tag.attributes.erase(it);
										//	next = false;
										//	ctx_.is_project_dirty = true;
										//});

										//if (next)
										//{
										//	++it;
										//}
									}

									if (!new_name_candidate.empty())
									{
										auto node = tag.attributes.extract(new_name_candidate);
										node.key() = new_name;
										tag.attributes.insert(std::move(node));
									}
									ImGui::EndTable();
								}
								ImGui::PopStyleColor(2); //3
							}
						});
						widgets::end_collapsible();
					}

					ImGui::PopID();


					//ImGui::TableNextColumn();

					if (open_color_picker)
					{
						color_ref_ = it;
						ctx_.color_picker.set_color(ImGui::ColorConvertU32ToFloat4(tag.color));
						ctx_.color_picker.set_opened(true);
					}

					if (it != tags.end())
					{
						++it;
					}
				}

				if (filter_passes == 0)
				{
					ui::centered_text("No matching tags found...", ImGui::GetContentRegionAvail());
				}

				if (ctx_.color_picker.render("##TagColorPicker") and color_ref_ != tags.end())
				{
					color_ref_->color = ImGui::ColorConvertFloat4ToU32(ctx_.color_picker.color());
					ctx_.is_project_dirty = true;
				}
			}
			ImGui::EndChild();
		}

		if (add_tag_attribute_popup_ != nullptr)
		{
			add_tag_attribute_popup_->open_and_render(!add_tag_attribute_popup_->is_open());
			if (!add_tag_attribute_popup_->is_open())
			{
				add_tag_attribute_popup_.reset();
			}
		}

		if (new_tag_popup_ != nullptr)
		{
			new_tag_popup_->open_and_render(!new_tag_popup_->is_open());
			if (!new_tag_popup_->is_open())
			{
				new_tag_popup_.reset();
			}
		}

		if (rename_tag_popup_ != nullptr)
		{
			rename_tag_popup_->open_and_render(!rename_tag_popup_->is_open());
			if (!rename_tag_popup_->is_open())
			{
				rename_tag_popup_.reset();
			}
		}

		if (delete_tag_popup_ != nullptr)
		{
			delete_tag_popup_->open_and_render(!delete_tag_popup_->is_open());
			if (!delete_tag_popup_->is_open())
			{
				delete_tag_popup_.reset();
			}
		}
	}
}
