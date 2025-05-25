#pragma once
#include <random>
#include <type_traits>
#include <limits>

namespace vt::utils
{
	class random
	{
		random() = delete;

	private:
		static std::random_device rd;
		static std::mt19937_64 gen;

	public:
		template<typename type> static type get(type min = std::numeric_limits<type>::min(), type max = std::numeric_limits<type>::max())
		{
			if constexpr (std::is_floating_point_v<type>)
			{
				std::uniform_real_distribution<type> distribution(min, max);
				return distribution(rd);
			}
			else
			{
				std::uniform_int_distribution<type> distribution(min, max);
				return distribution(rd);
			}			
		}

		template<typename type> static type get_from_zero(type max = 1)
		{
			return get<type>(0, max);
		}
	};

	inline std::random_device random::rd{};
	inline std::mt19937_64 random::gen = std::mt19937_64(random::rd());
}
