#pragma once
#include "simple_attribute_factory.hpp"
#include <attributes/impl/shape.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_factory : public simple_attribute_factory<shape_type>
	{
	public:
		shape_factory(const std::string& name) : simple_attribute_factory<shape_type>{ name } {}
	};
}
