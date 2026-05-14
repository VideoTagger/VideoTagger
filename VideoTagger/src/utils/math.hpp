#pragma once

namespace vt::math
{
	/**
	 * @brief Normalizes a value from one range to another
	 * @param[in] value The value to normalize
	 * @param[in] min The minimum of the original range
	 * @param[in] max The maximum of the original range
	 * @param[in] new_min The minimum of the new range
	 * @param[in] new_max The maximum of the new range
	 * @tparam type_out The type of the normalized value in the new range
	 * @tparam type_in The type of the input value, minimum and maximum values
	 * 
	 * @return Normalized value in the new range
	 */
	template<typename type_out, typename type_in>
	inline type_out normalize(type_in value, type_in min, type_in max, type_out new_min, type_out new_max)
	{
		return static_cast<type_out>(new_min + (value - min) * (new_max - new_min) / (max - min));
	}

	/**
	 * @brief Linearly interpolates between two values based on the alpha value
	 * @param[in] start The starting value
	 * @param[in] end The ending value
	 * @param[in] alpha The interpolation factor in range [0.0f, 1.0f]
	 * @tparam type The type of the values to interpolate
	 * 
	 * @return Interpolated value between start and end
	 */
	template<typename type>
	inline constexpr type lerp(const type& start, const type& end, float alpha)
	{
		return type((1.f - alpha) * start + alpha * end);
	}

	/**
	 * @brief Scale a value to the given range
	 * @param value The value to scale
	 * @param value_min The minimum of the current value range
	 * @param value_max The maximum of the current value range
	 * @param target_min The minimum of the target value range
	 * @param target_max The maximum of the target value range
	 * @tparam from_type The type of the input value and its range
	 * @tparam to_type The type of the output value and its range
	 * @return The value scaled to the target range
	 */
	template<typename from_type, typename to_type>
	inline constexpr to_type scale_value(from_type value, from_type value_min, from_type value_max, to_type target_min, to_type target_max, bool clamp)
	{
		if (clamp)
		{
			value = std::clamp(value, value_min, value_max);
		}

		return target_min + ((value - value_min) * (target_max - target_min)) / (value_max - value_min);
	}

	/**
	 * @brief Scale a 2D vector to the given range
	 * @param value The vector to scale
	 * @param value_min The minimum of the current vector range
	 * @param value_max The maximum of the current vector range
	 * @param target_min The minimum of the target vector range
	 * @param target_max The maximum of the target vector range
	 * @tparam from_type The type of the input vector and its range. Must support operator[] and have at least 2 elements.
	 * @tparam to_type The type of the output vector and its range. Must support operator[] and have at least 2 elements.
	 * @return The vector scaled to the target range
	 */
	template<typename from_type, typename to_type>
	inline constexpr to_type scale_vec2(from_type value, from_type value_min, from_type value_max, to_type target_min, to_type target_max, bool clamp)
	{
		return
		{
			scale_value(value[0], value_min[0], value_max[0], target_min[0], target_max[0], clamp),
			scale_value(value[1], value_min[1], value_max[1], target_min[1], target_max[1], clamp)
		};
	}
}
