#pragma once
#include <vector>
#include <string>
#include <optional>
#include <attributes/impl/shape.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	struct shape_interpolator_data
	{
		shape_type shape;
		timestamp ts;
	};
}

namespace vt::impl
{
	/**
	 * @brief Base class for shape interpolator
	 * 
	 * @tparam shape_type Type of shape to interpolate
	 */
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_interpolator
	{
	public:
		shape_interpolator(const std::string& name) : name_{ name } {}
		virtual ~shape_interpolator() = default;

	private:
		std::string name_;

	public:
		/// @return The name of the tracker
		const std::string& name() const
		{
			return name_;
		}

		/// @return How many data points the interpolator requires to provide the best accuracy.
		virtual size_t data_point_count() const = 0;

		/**
		 * @brief Interpolate the shape instance at the given moment
		 * 
		 * @param interpolation_data Data points to use for interpolation
		 * @param current_ts Timestamp at which to make the interpolation
		 */
		virtual std::optional<shape_type> interpolate(const std::vector<shape_interpolator_data<shape_type>>& interpolation_data, timestamp current_ts) = 0;
	};
}
