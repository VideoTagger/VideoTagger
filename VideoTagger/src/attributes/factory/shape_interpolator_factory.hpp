#pragma once
#include <attributes/impl/shape_interpolator.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_interpolator_factory
	{
	public:
		shape_interpolator_factory(const std::string& name) : name_{ name } {}

	private:
		std::string name_;

	public:
		const std::string& name() const
		{
			return name_;
		}

		virtual std::unique_ptr<impl::shape_interpolator<shape_type>> new_shape_interpolator() = 0;
	};
}
