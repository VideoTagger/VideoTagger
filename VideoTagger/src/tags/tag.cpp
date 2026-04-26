#include "pch.hpp"
#include "tag.hpp"

#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text_input.hpp>
#include "tag_timeline.hpp"
#include <core/app_context.hpp>

namespace vt
{
	void tag_attribute_instance::draw(const std::string& name, const tag_attribute& attribute, bool& dirty_flag)
	{
		const theme& theme = ctx_.current_theme;
		const auto& style = ImGui::GetStyle();
		bool has_value = this->has_value();

		bool selected = (ctx_.get_selected_attribute() == this);

		auto cpos = ImGui::GetCursorPos();
		if (ImGui::Selectable("##TagAttributeInstanceSelectable", selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns, { 0.f, ImGui::GetTextLineHeightWithSpacing() + style.ItemSpacing.y }))
		{
			ctx_.set_selected_attribute(this);
		}
		ImGui::SetCursorPos(cpos);

		widgets::frame_color_indicator(3.f, tag_attribute::type_color(attribute.type_));
		ImGui::SameLine(style.ItemSpacing.x);
		//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		//ImGui::InputText("##AttributeName", (std::string*)&name, ImGuiInputTextFlags_ReadOnly);
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(style.Colors[has_value ? ImGuiCol_Text : ImGuiCol_TextDisabled], "%s", name.c_str());
		if (ImGui::BeginPopupContextItem("##AttributeCtxMenu"))
		{
			if (ImGui::MenuItem(fmt::format("{} Reset", icons::reset).c_str(), nullptr, nullptr, has_value))
			{
				clear_value();
				dirty_flag = true;
				has_value = false;
			}
			if (ImGui::MenuItem(fmt::format("{} Mark Modified", icons::edit).c_str(), nullptr, nullptr, !has_value))
			{
				dirty_flag = true;
				has_value = true;
			}
			ImGui::EndPopup();
		}

		std::string var_tooltip = fmt::format("Type: {}", utils::string::to_titlecase(tag_attribute::type_str(attribute.type_)));
		ui::tooltip(var_tooltip);
		ImGui::NextColumn();

		switch (attribute.type_)
		{
			case tag_attribute::type::bool_:
			{
				bool v{};
				if (has_value)
				{
					if (!has<bool>()) *this = bool{};
					v = get<bool>();
				}
				if (ImGui::Checkbox("##AttributeCheckbox", &v))
				{
					*this = v;
				}
			}
			break;
			case tag_attribute::type::float_:
			{
				double v{};
				if (has_value)
				{
					if (!has<double>()) *this = double{};
					v = get<double>();
				}
				if (ImGui::DragScalar("##AttributeFloat", ImGuiDataType_Double, &v, 1.f, nullptr, nullptr, "%g", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat))
				{
					*this = v;
				}
			}
			break;
			case tag_attribute::type::integer:
			{
				int64_t v{};
				if (has_value)
				{
					if (!has<int64_t>()) *this = int64_t{};
					v = get<int64_t>();
				}
				if (ImGui::DragScalar("##AttributeInt", ImGuiDataType_S64, &v, 1.f, nullptr, nullptr, nullptr, ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat))
				{
					*this = v;
				}
			}
			break;
			case tag_attribute::type::string:
			{
				std::string v{};
				if (has_value)
				{
					if (!has<std::string>()) *this = std::string{};
					v = get<std::string>();
				}

				ui::text_input input("##AttributeString", v, "Empty");
				if (input.render())
				{
					*this = v;
				}
			}
			break;
			case tag_attribute::type::shape:
			{
				shape v{};
				if (has_value)
				{
					if (!has<shape>()) *this = shape{};
					v = get<shape>();
				}

				const auto& shapes = shape::types;
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
				float button_width = ImGui::CalcTextSize(shape::type_icon(shape::type::none)).x;
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.ItemSpacing.y);
				if (ImGui::BeginTable("##ShapeAttributeType", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV))
				{
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

					ImGui::TableNextColumn();

					bool will_fit = (ImGui::GetContentRegionAvail().x - (button_width + 2 * style.FramePadding.x) * shapes.size() >= 0);
					//ImGui::PushStyleColor(ImGuiCol_ChildBg, theme.get_float4(theme_color::background_base));
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
					if (ImGui::BeginChild("##ScrollableShapeTypeList", { ImGui::GetContentRegionAvail().x, (will_fit ? ImGui::GetTextLineHeight() : ImGui::GetTextLineHeight() + style.ScrollbarSize) + 2.f * style.FramePadding.y }, 0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings))
					{
						for (size_t i = 0; i < shapes.size(); ++i)
						{
							auto type = (shape::type)i;
							if (ui::icon_toggle_button(fmt::format("{}", shape::type_icon(type)), v.get_type() == type))
							{
								if (v.get_type() != type)
								{
									v.set_type(type);
									*this = v;
									dirty_flag = true;
								}
							}
							std::string btn_tooltip = utils::string::to_titlecase(shape::type_str(type));
							ui::tooltip(btn_tooltip);
							if (i + 1 < shapes.size())
							{
								ImGui::SameLine();
							}
						}
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::TableNextColumn();
					if (ui::icon_toggle_button(icons::interpolate, v.interpolate))
					{
						v.interpolate = !v.interpolate;
						*this = v;
						dirty_flag = true;
					}
					ui::tooltip(v.interpolate ? "Interpolation: On" : "Interpolation: Off");
					ImGui::EndTable();
					//ImGui::PopStyleColor();
				}
				ImGui::PopStyleVar(2);
			}
			break;
		}

		if (ImGui::IsWindowHovered() and ImGui::IsMouseClicked(0) and !ImGui::IsAnyItemHovered())
		{
			ctx_.set_selected_attribute(nullptr);
		}
		ImGui::NextColumn();
	}

	static void draw_tag_attribute(const std::string& name, tag_attribute& attr, const std::function<void(const std::string&)>& on_name_change, const std::function<void(tag_attribute::type)>& on_type_change, const std::function<void()>& on_delete)
	{
		const auto& style = ImGui::GetStyle();

		bool selected{};
		bool row_hovered = widgets::table_hovered_row_style();

		ImGui::PushID(&attr);
		ImGui::TableNextColumn();
		ImGui::BeginGroup();
		widgets::color_indicator(3.f, tag_attribute::type_color(attr.type_), 1.f);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		std::string new_name = name;
		if (ImGui::InputText("##TagAttributeName", &new_name, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			on_name_change(new_name);
		}
		ImGui::TableNextColumn();

		int current_type = (int)attr.type_;
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::Combo("##TagAttributeType", &current_type, tag_attribute::types_str, (int)tag_attribute::type_count))
		{
			on_type_change((tag_attribute::type)current_type);
		}

		switch (attr.type_)
		{
			case tag_attribute::type::bool_: ui::tooltip("Value: True/False"); break;
			case tag_attribute::type::float_: ui::tooltip("Value: Float (64 bit)"); break;
			case tag_attribute::type::integer: ui::tooltip("Value: Integer (64 bit)"); break;
			case tag_attribute::type::string: ui::tooltip("Value: Text"); break;
			case tag_attribute::type::shape:
			{
				std::string shapes;
				size_t i{};
				for (auto type : shape::types)
				{
					shapes += utils::string::to_titlecase(shape::type_str(type));
					if (++i < shape::types.size())
					{
						shapes += "/";
					}
				}
				ui::tooltip(fmt::format("Value: {}", shapes));
			}
			break;
		}

		ImGui::EndGroup();
		if (ImGui::BeginPopupContextItem("##TagAttributeCtxMenu"))
		{
			std::string menu_name = fmt::format("{} Delete", icons::delete_);
			if (ImGui::MenuItem(menu_name.c_str()))
			{
				on_delete();
			}
			ImGui::EndPopup();
		}
		if (row_hovered and ImGui::IsMouseClicked(1))
		{
			ImGui::OpenPopup("##TagAttributeCtxMenu");
		}
		ImGui::PopID();
	}

	bool tag::draw_attributes(bool& dirty_flag, const std::function<void()>& on_add_new)
	{
		bool modifiable = !ui::is_item_disabled();

		static constexpr float table_border_size = 1.f; //FIXME: This is currently hardcoded in ImGui, change this when ImGui uses different border size
		if (ImGui::BeginTable("##Attributes", 2, ImGuiTableFlags_BordersOuter, { ImGui::GetContentRegionAvail().x - table_border_size, 0 }))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");

			ImGui::TableNextColumn();
			if (modifiable)
			{
				if (ui::icon_button(icons::add))
				{
					on_add_new();
				}
				ImGui::SameLine();
			}
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Attributes");

			if (!attributes.empty())
			{
				ImGui::BeginDisabled();
				ImGui::TableHeadersRow();
				ImGui::EndDisabled();
			}
			std::string new_name_candidate;
			std::string new_name;

			for (auto it = attributes.begin(); it != attributes.end();)
			{
				bool next = true;
				auto& [name, attr] = *it;
				ImGui::TableNextRow();
				draw_tag_attribute(name, attr,
				[&new_name_candidate, &new_name, &name, modifiable](const std::string& nname)
				{
					if (!modifiable) return;
					new_name_candidate = name;
					new_name = nname;
				},
				[&attr, modifiable](const tag_attribute::type new_type)
				{
					if (!modifiable) return;
					attr.type_ = new_type;
				},
				[this, &dirty_flag, &it, &name, &next, modifiable]()
				{
					if (!modifiable) return;
					it = attributes.erase(it);
					next = false;
					dirty_flag = true;
				});

				if (next)
				{
					++it;
				}
			}

			if (!new_name_candidate.empty())
			{
				auto node = attributes.extract(new_name_candidate);
				node.key() = new_name;
				attributes.insert(std::move(node));
			}
			ImGui::EndTable();
		}
		return false;
	}

	bool tag::draw_attribute_instances(const tag_segment& selected_segment, video_id_t video_id, bool& dirty_flag) const
	{
		const auto& style = ImGui::GetStyle();
		auto flags = ui::is_item_disabled() ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
		if (ui::is_item_disabled())
		{
			ImGui::SetNextItemOpen(false, ImGuiCond_Appearing);
		}
		bool visible = widgets::begin_collapsible("##Attributes", "Attributes", flags, icons::attribute);
		if (visible)
		{
			auto selected_attr_inst = ctx_.get_selected_attribute();

			ui::card([&]()
			{
				ImGui::Columns(2);
				{
					int i{};
					bool contains_selected_attr = false;
					for (auto& [name, attr] : attributes)
					{
						auto& attr_inst = selected_segment.attributes[video_id][name];

						ImGui::PushID(i++);
						attr_inst.draw(name, attr, dirty_flag);
						ImGui::PopID();

						if (&attr_inst == selected_attr_inst)
						{
							contains_selected_attr = true;
						}
					}

					if (selected_attr_inst != nullptr and !contains_selected_attr)
					{
						ctx_.set_selected_attribute(nullptr);
					}
				}
				ImGui::Columns();
			});
			widgets::end_collapsible();
		}
		return visible;
	}

	nlohmann::ordered_json tag::serialize() const
	{
		nlohmann::ordered_json json;
		json["name"] = name;
		json["color"] = utils::color::to_string(color);

		auto json_attributes = nlohmann::ordered_json::array();
		for (const auto& [name, attr] : attributes)
		{
			json_attributes.push_back(attr->serialize());
		}
		json["attributes"] = json_attributes;
		return json;
	}

	void tag::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("name") or !json.contains("color")) return;

		name = json["name"];
		auto col_str = json["color"].get<std::string>();
		utils::color::parse_string(col_str, color);

		if (json.contains("attributes"))
		{
			for (const auto& attr_data : json["attributes"])
			{
				auto attr = ctx_.attr_registry.deserialize_attribute(attr_data);
				auto attr_name = attr->name();
				attributes.try_emplace(std::move(attr_name), std::move(attr));
			}
		}
	}

	std::unique_ptr<impl::attribute_instance> tag::deserialize_attribute_instance(const nlohmann::ordered_json& json) const
	{
		if (!json.contains("name") or !json.contains("type"))
		{
			debug::error("Invalid attribute instance JSON structure, missing 'name' or 'type' field");
			return nullptr;
		}

		std::string attr_name = json["name"];
		auto attr_it = attributes.find(attr_name);
		if (attr_it == attributes.end())
		{
			debug::error("No attribute with name '{}' found in tag '{}'", attr_name, name);
			return nullptr;			
		}

		return attr_it->second->deserialize_instance(json);
	}
}
