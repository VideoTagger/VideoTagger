#pragma once
#include <random>
#include <type_traits>

namespace vt::utils
{
	class random
	{
		random() = delete;

	private:
		static std::random_device rd;
		static std::mt19937_64 gen;

	public:
		template<typename type> static type get(type min, type max)
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
		template<typename type> static type get(type max = 1.0f)
		{
			return get<type>(0, max);
		}
	};

	std::random_device random::rd{};
	std::mt19937_64 random::gen = std::mt19937_64(random::rd());
}