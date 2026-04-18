#pragma once
#include <tags/impl/attribute.hpp>
#include <tags/impl/attribute_factory.hpp>

#include <tags/attributes/simple_attribute.hpp>
#include <tags/attributes/simple_attribute_instance.hpp>

namespace vt
{
	template<typename attribute_type>
	class simple_attribute_factory : public impl::attribute_factory
	{
		virtual std::unique_ptr<impl::attribute> create() override
		{
			return std::make_unique<simple_attribute<attribute_type>>();
		}

		virtual std::unique_ptr<impl::attribute_instance> instantiate() override
		{
			return std::make_unique<simple_attribute_instance<attribute_type>>();
		}
	};
}
