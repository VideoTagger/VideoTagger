#pragma once
#include <string>
#include <memory>
#include <attributes/impl/shape.hpp>
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <attributes/core/shape_attribute_instance.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_attribute : public impl::attribute
	{
	public:
		shape_attribute(impl::attribute_factory* factory, const std::string& name) : attribute{ factory, name } {}

	public:
		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<shape_attribute_instance<shape_type>>(this);
		}
	};
}
