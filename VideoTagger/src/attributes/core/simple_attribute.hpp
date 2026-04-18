#pragma once
#include <attributes/impl/attribute.hpp>
#include <attributes/core/simple_attribute_instance.hpp>

namespace vt
{
	template<typename type>
	struct simple_attribute : public impl::attribute
	{
		simple_attribute(impl::attribute_factory* factory, const std::string& name) : impl::attribute{ factory, name } {}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<simple_attribute_instance<type>>(this);
		}
	};
}
