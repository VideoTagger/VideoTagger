#pragma once
#include <attributes/impl/shape_interpolator.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	/// @brief Shape predictor always returning the shape it was initialized with
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class static_shape_interpolator : public impl::shape_interpolator<shape_type>
	{
	public:
		static_shape_interpolator(const std::string& name) : impl::shape_interpolator<shape_type>{ name } {}

	public:
		virtual size_t data_point_count() const
		{
			return 1;
		}

		static std::optional<shape_type> interpolate(const shape_type& shape_instance)
		{
			return shape_instance;
		}

		virtual std::optional<shape_type> interpolate(const std::vector<shape_interpolator_data<shape_type>>& interpolation_data, timestamp current_ts) override
		{
			if (interpolation_data.empty()) return std::nullopt;

			return interpolate(interpolation_data[0].shape);
		}
	};
}
