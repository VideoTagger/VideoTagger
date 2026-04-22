#pragma once
#include <memory>
#include <string>
#include "simple_attribute_factory.hpp"
#include <attributes/impl/shape.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_factory : public impl::attribute_factory
	{
	public:
		shape_attribute_factory(const std::string& name) : impl::attribute_factory{ name } {}

	public:
		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) override
		{
			return std::make_unique<shape_attribute<shape_type>>(this, name);
		}
	};
}
