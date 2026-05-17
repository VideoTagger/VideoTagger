#pragma once
#include <attributes/factory/shape_interpolator_factory.hpp>
#include <attributes/predictors/linear_shape_predictor.hpp>

namespace vt
{
	template<typename shape_type>
	class linear_shape_predictor_factory : public shape_interpolator_factory<shape_type>
	{
	public:
		linear_shape_predictor_factory(const std::string& name) : shape_interpolator_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::shape_interpolator<shape_type>> new_shape_interpolator() override
		{
			return std::make_unique<linear_shape_predictor<shape_type>>(this->name());
		}
	};
}
