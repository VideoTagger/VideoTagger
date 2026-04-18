#pragma once
#include <type_traits>
#include <impl/resettable.hpp>
#include <impl/serializable.hpp>

namespace vt::impl
{
	struct attribute_instance : public resettable, public serializable
	{
		virtual ~attribute_instance() = default;

		virtual void reset() override {}

		template<typename type, typename = std::enable_if_t<std::is_base_of_v<attribute_instance, type>>>
		[[nodiscard]] type* as()
		{
			return reinterpret_cast<type*>(this);
		}
	};
}
