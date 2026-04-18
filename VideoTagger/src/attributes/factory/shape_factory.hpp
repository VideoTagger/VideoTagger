#pragma once
#include "attribute_factory.hpp"
#include <attributes/impl/shape.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_factory : public impl::attribute_factory
	{
	public:
		shape_factory(const std::string& name) : impl::attribute_factory{ name } {}

	public:
		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) = 0;
	};
}
