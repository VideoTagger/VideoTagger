#pragma once

namespace vt::math
{
	template<typename type_out, typename type_in>
	static inline type_out normalize(type_in value, type_in min, type_in max, type_out new_min, type_out new_max)
	{
		return static_cast<type_out>(new_min + (value - min) * (new_max - new_min) / (max - min));
	}
}
