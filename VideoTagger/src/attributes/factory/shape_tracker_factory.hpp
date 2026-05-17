#pragma once
#include <attributes/factory/shape_predictor_factory.hpp>
#include <attributes/impl/shape_tracker.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_tracker_factory : public shape_predictor_factory<shape_type>
	{
	public:
		shape_tracker_factory(const std::string& name) : shape_predictor_factory<shape_type>{ name } {}

		virtual std::unique_ptr<impl::shape_tracker<shape_type>> new_shape_tracker() = 0;

		virtual std::unique_ptr<impl::shape_predictor<shape_type>> new_shape_predictor() override
		{
			return new_shape_tracker();
		}
	};
}
