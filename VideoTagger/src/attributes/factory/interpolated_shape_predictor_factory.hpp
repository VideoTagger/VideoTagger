#pragma once
#include <attributes/factory/shape_predictor_factory.hpp>
#include <attributes/impl/interpolated_shape_predictor.hpp>

namespace vt
{
	template<typename shape_type>
	class interpolated_shape_predictor_factory : public shape_predictor_factory<shape_type>
	{
	public:
		interpolated_shape_predictor_factory(const std::string& name) : shape_predictor_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::interpolated_shape_predictor<shape_type>> new_shape_interpolator() = 0;
	};
}
