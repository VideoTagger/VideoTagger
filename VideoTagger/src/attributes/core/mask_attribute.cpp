#include "mask_attribute.hpp"
#include <attributes/factory/mask_attribute_factory.hpp>

namespace vt
{
	mask_attribute::mask_attribute(impl::attribute_factory* factory, const std::string& name) : shape_attribute<mask_shape>{ factory, name } {}

	void mask_attribute::register_event_listeners()
	{
		tool_register_handle_ = ctx_.add_event_listener<toolbar_register_request_event>([this](const toolbar_register_request_event& event)
		{
			if (!ctx_.session.is_one_segment_selected()) return;

			auto segment_opt = ctx_.session.any_selected_segment();
			const auto& [tag_name, segment] = *segment_opt;
			const auto& tag_data = ctx_.current_project->tags.at(tag_name);
			auto attribute_it = tag_data.attributes.find(name());
			if (attribute_it == tag_data.attributes.end()) return;

			debug::log("Registering toolbar tool with type: '{}' for shape attribute '{}'", type_name(), name());

			auto shape_factory = reinterpret_cast<mask_attribute_factory*>(factory());
			auto& tool_group = ctx_.session.toolbar.group("shapes");

			auto tools = shape_factory->new_tools(tag_data, name());
			int64_t sort_index = tool_group.size();
			for (size_t i = 0; i < tools.size(); ++i)
			{
				tool_group
					.add_tool(event.source(), shape_factory->tool_specification(i), std::move(tools[i]))
					.set_sort_index(-sort_index--);
			}
		});
	}
}
