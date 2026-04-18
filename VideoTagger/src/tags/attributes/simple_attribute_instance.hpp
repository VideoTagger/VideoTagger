#pragma once
#include <tags/impl/attribute_instance.hpp>

namespace vt
{
	template<typename type>
	struct simple_attribute_instance : public impl::attribute_instance
	{
	public:
		simple_attribute_instance(const type& value = {}) : value_{ value } {}

	private:
		type value_;

	public:
		virtual void reset() override
		{
			value_ = {};
		}

		type& value()
		{
			return value_;
		}

		const type& value() const
		{
			return value_;
		}
	};
}
