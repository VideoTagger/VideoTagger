#include "add_tag_attribute_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/text.hpp>
#include <ui/widgets/button_bar.hpp>

namespace vt::ui
{
	add_tag_attribute_popup::add_tag_attribute_popup(event_source source, const std::string& tag_name, std::optional<bool*> open) :
		modal_popup("add-tag-attribute-popup", open), event_source_{ source }, tag_name_{ tag_name },
		attribute_input_{ "##TagAttributeNameInput", "Attribute Name...", [](const std::string& input) -> std::optional<std::string>
		{
			if (input.empty())
			{
				return "Name cannot be empty";
			}

			return std::nullopt;
		} },
		type_combo_{ "##TagAttributeTypeCombo", ctx_.attr_registry.title_attribute_names(), 0 }
	{}

	void add_tag_attribute_popup::on_display()
	{
		set_display_name(ctx_.lang->get("popup.add_tag_attribute.title"));
	}

	void add_tag_attribute_popup::on_render()
	{
		attribute_input_.render_with_label("Name");
		type_combo_.render_with_label("Type");

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, attribute_input_.is_valid(), [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				////TODO: should use an event
				auto attribute = ctx_.attr_registry.new_attribute(utils::string::to_lowercase(type_combo_.selected_item()), attribute_input_.input());
				ctx_.current_project->tags.at(tag_name_).attributes.try_emplace(attribute_input_.input(), std::move(attribute));
				ctx_.is_project_dirty = true;

				close();
				break;
			}
			default: close(); break;
			}
		});
	}
}
