#pragma once
#include <type_traits>
#include <attributes/impl/attribute.hpp>

namespace vt::impl
{
	template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::attribute, type>>>
	struct attribute_ref : public attribute_instance
	{
	public:
		attribute_ref(type* ref) : attribute_instance{ ref } {}
		virtual ~attribute_ref() = default;

	public:
		type* attribute()
		{
			return reinterpret_cast<type*>(attribute_impl());
		}

		const type* attribute() const
		{
			return reinterpret_cast<const type*>(attribute_impl());
		}
	};
}
