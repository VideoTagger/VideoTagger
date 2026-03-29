#pragma once
#include <impl/resettable.hpp>

namespace vt::impl
{
	struct attribute_instance : public resettable
	{
		virtual ~attribute_instance() = default;
	};
}
