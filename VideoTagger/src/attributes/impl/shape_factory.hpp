#pragma once
#include "attribute_factory.hpp"
#include <tags/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_factory : public attribute_factory
	{
	public:
		virtual ~shape_factory() = default;

	public:
		virtual std::unique_ptr<impl::attribute> create() = 0;
		virtual std::unique_ptr<impl::attribute_instance> instantiate() = 0;
	};
}
