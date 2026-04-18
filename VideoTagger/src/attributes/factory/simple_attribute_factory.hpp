#pragma once
#include <attributes/impl/attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>

#include <attributes/core/simple_attribute.hpp>
#include <attributes/core/simple_attribute_instance.hpp>
namespace vt
{
	template<typename attribute_type>
	class simple_attribute_factory : public impl::attribute_factory
	{
	public:
		simple_attribute_factory(const std::string& name) : impl::attribute_factory{ name } {}

	public:
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
