#pragma once
#include <memory>
#include "attribute.hpp"
#include "attribute_instance.hpp"

namespace vt::impl
{
	class attribute_factory
	{
	public:
		virtual ~attribute_factory() = default;

		virtual std::unique_ptr<attribute> create() = 0;
		virtual std::unique_ptr<attribute_instance> instantiate() = 0;
	};
}
