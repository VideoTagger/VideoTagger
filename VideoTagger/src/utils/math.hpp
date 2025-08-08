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
}
