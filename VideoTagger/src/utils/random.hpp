#pragma once
#include <random>
#include <type_traits>
#include <limits>

namespace vt::utils
{
	using uuid = uint64_t;

	class random
	{
		random() = delete;

	private:
		static std::random_device rd;
		static std::mt19937_64 gen;

	public:
		/**
		 * @brief Generates a random value of the specified type within the given range [min, max]
		 * 
		 * @param[in] min The minimum value (inclusive) of the range. Defaults to the minimum value of the type
		 * @param[in] max The maximum value (inclusive) of the range. Defaults to the maximum value of the type
		 * @tparam type The integral or floating-point type of the random value to generate
		 * 
		 * @return Random value of the specified type within the given range [min, max]
		 */
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

		/**
		 * @brief Generates a random value of the specified type within the range [0, max]
		 * @param[in] max The maximum value (inclusive) of the range. Defaults to 1
		 * @tparam type The integral or floating-point type of the random value to generate
		 * 
		 * @return Random value of the specified type within the range [0, max]
		 */
		template<typename type> static type get_from_zero(type max = 1)
		{
			return get<type>(0, max);
		}

		/**
		 * @brief Generates a random UUID
		 * 
		 * @return Random UUID
		 */
		static uuid get_uuid()
		{
			return get<uuid>(1);
		}
	};

	inline std::random_device random::rd{};
	inline std::mt19937_64 random::gen = std::mt19937_64(random::rd());
}
