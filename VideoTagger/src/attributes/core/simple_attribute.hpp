#pragma once
#include <attributes/impl/attribute.hpp>
#include <attributes/core/simple_attribute_instance.hpp>
#include <attributes/impl/attribute_property_renderer.hpp>

namespace vt
{
	template<typename type>
	struct simple_attribute : public impl::attribute, public impl::attribute_property_renderer
	{
		simple_attribute(impl::attribute_factory* factory, const std::string& name) : impl::attribute{ factory, name } {}

		virtual bool render_instance_properties(std::unique_ptr<impl::attribute_instance>& instance)
		{
			const auto& name = instance->attribute_name();
			auto typed_inst = instance->as<simple_attribute_instance<type>>();
			return render_property(name, type_name(), typed_inst->value());
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<simple_attribute_instance<type>>(this);
		}
	};
}
