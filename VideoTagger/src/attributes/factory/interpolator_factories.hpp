#pragma once
#include <attributes/factory/shape_interpolator_factory.hpp>
#include <attributes/predictors/dummy_shape_interpolator.hpp>
#include <attributes/predictors/linear_shape_interpolator.hpp>

namespace vt
{
	template<typename shape_type>
	class dummy_shape_interpolator_factory : public shape_interpolator_factory<shape_type>
	{
	public:
		dummy_shape_interpolator_factory(const std::string& name) : shape_interpolator_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::shape_interpolator<shape_type>> new_shape_interpolator() override
		{
			return std::make_unique<dummy_shape_interpolator<shape_type>>(this->name());
		}
	};

	template<typename shape_type>
	class linear_shape_interpolator_factory : public shape_interpolator_factory<shape_type>
	{
	public:
		linear_shape_interpolator_factory(const std::string& name) : shape_interpolator_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::shape_interpolator<shape_type>> new_shape_interpolator() override
		{
			return std::make_unique<linear_shape_interpolator<shape_type>>(this->name());
		}
	};
}
