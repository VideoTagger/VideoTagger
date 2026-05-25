#pragma once
#include <string>
#include <memory>
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <attributes/core/shape_attribute_instance.hpp>
#include <attributes/impl/attribute_property_renderer.hpp>
#include <core/app_context.hpp>
#include <events/toolbar/toolbar_register_request_event.hpp>
#include <attributes/tools/shape_tool.hpp>
#include <ui/toolbar/toolbar_group.hpp>

namespace vt
{
	template<typename shape_type, typename>
	class shape_attribute_factory;

	template<typename shape_type>
	class shape_attribute : public impl::attribute, public impl::attribute_property_renderer
	{
	public:
		shape_attribute(impl::attribute_factory* factory, const std::string& name) : attribute{ factory, name }, tool_register_handle_{} {}

		virtual ~shape_attribute()
		{
			auto& dispatcher = ctx_.get_event_dispatcher<toolbar_register_request_event>();
			dispatcher.remove_event_listener(tool_register_handle_);
		}

	protected:
		event_listener_handle tool_register_handle_;

	public:
		virtual bool render_instance_properties(std::unique_ptr<impl::attribute_instance>& instance)
		{
			auto* typed_inst = instance->as<shape_attribute_instance<shape_type>>();
			if (typed_inst == nullptr) return false;

			const auto& name = instance->attribute_name();

			return render_property(name, type_name(), typed_inst->regions());
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<shape_attribute_instance<shape_type>>(this);
		}

		virtual void on_init() override
		{
			register_event_listeners();
		}

	protected:
		virtual void register_event_listeners()
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

				auto shape_factory = reinterpret_cast<shape_attribute_factory<shape_type>*>(factory());
				auto& tool_group = ctx_.session.toolbar.group("shapes");
				tool_group.add_tool(event.source(), shape_factory->tool_specification(), std::move(shape_factory->new_tool(tag_data, name())));
			});
		}
	};
}
