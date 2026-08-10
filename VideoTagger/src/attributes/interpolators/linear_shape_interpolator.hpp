#pragma once
#include <array>
#include <attributes/impl/shape_interpolator.hpp>
#include "static_shape_interpolator.hpp"
#include <utils/math.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	/// @brief Shape predictor using linear interpolation
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class linear_shape_interpolator : public impl::shape_interpolator<shape_type>
	{
	public:
		linear_shape_interpolator(const std::string& name) : impl::shape_interpolator<shape_type>{ name } {}

	public:
		virtual size_t data_point_count() const
		{
			return 2;
		}

		static std::optional<shape_type> interpolate(const shape_type& start_shape, timestamp start_ts, const shape_type& end_shape, timestamp end_ts, timestamp current_ts)
		{
			return math::shape_lerp<shape_type>(start_shape, end_shape, static_cast<float>((current_ts - start_ts).total_nanoseconds.count()) / (end_ts - start_ts).total_nanoseconds.count());
		}

		virtual std::optional<shape_type> interpolate(const std::vector<shape_interpolator_data<shape_type>>& interpolation_data, timestamp current_ts)
		{
			if (interpolation_data.empty()) return std::nullopt;

			if (interpolation_data.size() == 1) return static_shape_interpolator<shape_type>::interpolate(interpolation_data[0].shape);

			return interpolate(interpolation_data[0].shape, interpolation_data[0].ts, interpolation_data[1].shape, interpolation_data[1].ts, current_ts);
		}
	};
}
