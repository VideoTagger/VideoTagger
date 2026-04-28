#pragma once
#include <string>
#include <memory>
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <attributes/core/shape_attribute_instance.hpp>
#include <attributes/impl/attribute_property_renderer.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_attribute : public impl::attribute, public impl::attribute_property_renderer
	{
	public:
		shape_attribute(impl::attribute_factory* factory, const std::string& name) : attribute{ factory, name } {}

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
	};
}
