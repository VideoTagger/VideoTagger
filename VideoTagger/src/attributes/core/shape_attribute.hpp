#pragma once
#include <string>
#include <memory>
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <attributes/core/shape_attribute_instance.hpp>
#include <attributes/impl/attribute_property_renderer.hpp>
#include <core/app_context.hpp>
#include <events/toolbar/toolbar_register_request_event.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_attribute : public impl::attribute, public impl::attribute_property_renderer
	{
	public:
		shape_attribute(impl::attribute_factory* factory, const std::string& name) : attribute{ factory, name }, tool_register_handle_{}
		{
			register_event_listeners();
		}

		virtual ~shape_attribute()
		{
			auto& dispatcher = ctx_.get_event_dispatcher<toolbar_register_request_event>();
			dispatcher.remove_event_listener(tool_register_handle_);
		}

	private:
		event_listener_handle tool_register_handle_;

	public:
		virtual bool render_instance_properties(std::unique_ptr<impl::attribute_instance>& instance)
		{
			const auto& name = instance->attribute_name();
			auto typed_inst = instance->as<shape_attribute_instance<shape_type>>();
			return render_property(name, type_name(), typed_inst->regions());
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<shape_attribute_instance<shape_type>>(this);
		}
	private:
		void register_event_listeners()
		{
			tool_register_handle_ = ctx_.add_event_listener<toolbar_register_request_event>([this](const toolbar_register_request_event& event)
			{
				const auto& selected_segments = ctx_.session.selected_segments();
				bool is_one_segment_selected = ctx_.session.is_one_segment_selected();
				if (!is_one_segment_selected) return;

				for (const auto& [tag, segment_ids] : selected_segments)
				{
					const auto& selected_segment = *segment_ids.begin();
					auto& tag_data = ctx_.current_project->tags.at(tag);
					auto it = tag_data.attributes.find(name());
					if (it == tag_data.attributes.end()) return;

					debug::log("TODO: register toolbar tool for shape attribute {}", name());
				}
			});
		}
	};
}
