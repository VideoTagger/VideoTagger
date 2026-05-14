#pragma once
#include <attributes/factory/interpolated_shape_predictor_factory.hpp>
#include <attributes/predictors/dummy_shape_predictor.hpp>

namespace vt
{
	template<typename shape_type>
	class dummy_shape_predictor_factory : public interpolated_shape_predictor_factory<shape_type>
	{
	public:
		dummy_shape_predictor_factory(const std::string& name) : interpolated_shape_predictor_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::interpolated_shape_predictor<shape_type>> new_shape_interpolator() override
		{
			return std::make_unique<dummy_shape_predictor<shape_type>>(this->name());
		}

		virtual std::unique_ptr<impl::shape_predictor<shape_type>> new_shape_predictor() override
		{
			return new_shape_interpolator();
		}
	};
}
