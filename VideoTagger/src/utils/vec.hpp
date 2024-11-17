#pragma once
#include <array>

namespace vt::utils
{
	template<typename type, size_t dims>
	struct vec
	{
		std::array<type, dims> data{};

		constexpr type& at(size_t index)
		{
			return data[index];
		}

		constexpr const type& at(size_t index) const
		{
			return data[index];
		}

		constexpr type& operator[](size_t index)
		{
			return data[index];
		}
		
		constexpr const type& operator[](size_t index) const
		{
			return data[index];
		}
	};

	template<typename type>
	using vec2 = vec<type, 2>;

	template<typename type>
	using vec4 = vec<type, 4>;
}
