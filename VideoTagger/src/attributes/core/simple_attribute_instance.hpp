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

		virtual void render_properties() override
		{

		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			//nlohmann::ordered_json json = impl::attribute_ref<simple_attribute<type>>::serialize();
			nlohmann::ordered_json json;
			json["value"] = value();
			return json;
		}

		virtual void deserialize(const nlohmann::ordered_json& json) override
		{
			//impl::attribute_ref<simple_attribute<type>>::deserialize(json);
			if (json.contains("value"))
			{
				value_ = json["value"].get<type>();
			}
		}
	};
}
