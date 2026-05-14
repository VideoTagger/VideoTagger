#pragma once
#include <attributes/impl/attribute_instance.hpp>
#include <attributes/impl/attribute_ref.hpp>

namespace vt
{
	template<typename type>
	struct simple_attribute;

	template<typename type>
	struct simple_attribute_instance : public impl::attribute_ref<simple_attribute<type>>
	{
	public:
		simple_attribute_instance(simple_attribute<type>* ref, const type& value = {}) : impl::attribute_ref<simple_attribute<type>>{ ref }, value_{ value } {}

	private:
		type value_;

	public:
		void set_value(const type& value)
		{
			value_ = value;
		}

		type& value()
		{
			return value_;
		}

		const type& value() const
		{
			return value_;
		}

		virtual void reset() override
		{
			value_ = {};
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			return value_;
		}

		virtual void deserialize(const nlohmann::ordered_json& json) override
		{
			value_ = json.get<type>();
		}
	};
}
