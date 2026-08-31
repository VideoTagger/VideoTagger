#include "add_tag_attribute_popup.hpp"

#include <core/app_context.hpp>
#include <ui/widgets/text.hpp>
#include <ui/widgets/button_bar.hpp>
#include <events/attributes/attribute_add_request_event.hpp>
#include <utils/name_validators.hpp>

namespace vt::ui
{
	add_tag_attribute_popup::add_tag_attribute_popup(event_source source, const std::string& tag_name, std::optional<bool*> open) :
		modal_popup("add-tag-attribute-popup", open), event_source_{ source }, tag_name_{ tag_name },
		attribute_input_{ "##TagAttributeNameInput", "Attribute Name...", [this](const std::string& input) -> std::optional<std::string>
		{
			const auto& tag = ctx_.current_project->tags.at(tag_name_);
			auto validation_result = utils::basic_map_name_validate(input, tag.attributes);
			if (validation_result == utils::name_validation_result::ok) return std::nullopt;

			return utils::name_validation_result_to_string(validation_result, *ctx_.lang);
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
			{ 0, ctx_.lang->get("generic.confirm") },
			{ 1, ctx_.lang->get("generic.cancel") },
		};
		ui::button_bar<int>::render(buttons, attribute_input_.is_valid(), [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				ctx_.dispatch_event<attribute_add_request_event>(event_source_, tag_name_, attribute_input_.input(), utils::string::to_lowercase(type_combo_.selected_item()));
				close();
				break;
			}
			default: close(); break;
			}
		});
	}
}
