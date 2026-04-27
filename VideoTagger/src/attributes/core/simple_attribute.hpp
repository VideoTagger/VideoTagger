#pragma once
#include <attributes/impl/attribute.hpp>
#include <attributes/core/simple_attribute_instance.hpp>

namespace vt
{
	template<typename type>
	struct simple_attribute : public impl::attribute
	{
		simple_attribute(impl::attribute_factory* factory, const std::string& name) : impl::attribute{ factory, name } {}

		virtual bool render_instance_properties(std::unique_ptr<impl::attribute_instance>& instance)
		{
			const auto& name = instance->attribute_name();
			auto typed_inst = instance->as<simple_attribute_instance<type>>();
			return render_property(name, typed_inst->value());
		}

		template<typename value_type>
		bool render_property(const std::string& name, value_type& value)
		{
			static constexpr auto line_width = 3.f;
			const auto& style = ImGui::GetStyle();

			bool was_modified{};
			bool has_value = true;
			auto attr_color = ctx_.attr_registry.get_attr_spec(type_name())->color;

			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			widgets::color_indicator(3.f, attr_color, 1.f, ImGui::GetFrameHeight());
			//ui::horizontal_item_spacer(line_width);
			ImGui::SameLine();

			ImGui::TextColored(style.Colors[has_value ? ImGuiCol_Text : ImGuiCol_TextDisabled], "%s", name.c_str());

			//auto draw_list = ImGui::GetWindowDrawList();
			//ImGuiTable* table = ImGui::GetCurrentTable();
			////TODO: Move this into ui helpers
			////left side color line
			//auto column = table->CurrentColumn;
			//if (table != nullptr)
			//{
			//	ImRect cell_rect
			//	{
			//		table->Columns[column].MinX,
			//		table->RowPosY1,
			//		table->Columns[column].MaxX,
			//		table->RowPosY2
			//	};

			//	cell_rect.Min.x += style.CellPadding.x + (line_width + style.ItemSpacing.x) / 2.f;
			//	cell_rect.Max.y = cell_rect.Min.y + ImGui::GetFrameHeight() + style.CellPadding.y * 2.f;
			//	cell_rect.Max.x = cell_rect.Min.x + line_width;
			//	cell_rect.Max.y -= style.CellPadding.y;

			//	draw_list->AddRectFilled(cell_rect.Min, cell_rect.Max, attr_color, 3.f);
			//}

			ImGui::TableNextColumn();
			auto prop_id = fmt::format("##Property:{}", name);
			//TODO: Add more supported types if needed
			if constexpr (std::is_same_v<value_type, bool>)
			{
				was_modified = ui::checkbox(prop_id, value);
			}
			else if constexpr (std::is_integral_v<value_type>)
			{
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				was_modified = ImGui::InputScalar(prop_id.c_str(), ImGuiDataType_S64, &value);
				ImGui::PopItemWidth();
			}
			else if constexpr (std::is_floating_point_v<value_type>)
			{
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				was_modified = ImGui::InputScalar(prop_id.c_str(), ImGuiDataType_Double, &value);
				ImGui::PopItemWidth();
			}
			else if constexpr (std::is_same_v<value_type, std::string>)
			{
				ui::text_input input(prop_id, value, "Empty");
				input.set_width(-1.f);
				was_modified = input.render();
				if (was_modified)
				{
					value = input.trimmed_input();
				}
			}
			return was_modified;
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<simple_attribute_instance<type>>(this);
		}
	};
}
