#pragma once
#include <type_traits>
#include <attributes/impl/attribute.hpp>

namespace vt::impl
{
	template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::attribute, type>>>
	struct attribute_ref : public attribute_instance
	{
	public:
		attribute_ref(type* ref) : ref_{ ref } {}
		virtual ~attribute_ref() = default;

	private:
		type* ref_;

	public:
		type* attribute()
		{
			return ref_;
		}

		const type* attribute() const
		{
			return ref_;
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			nlohmann::ordered_json result;
			result["type"] = ref_->type_name();
			return result;
		}
	};
}
