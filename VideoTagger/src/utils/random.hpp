#pragma once
#include <random>
#include <mutex>
#include <type_traits>
#include <limits>
#include <SDL.h>

namespace vt::utils
{
	using uuid = uint64_t;

	class random
	{
		random() = delete;

	private:
		static std::random_device rd;
		static std::mt19937_64 gen;
		static std::mutex mutex_;

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

		/**
		 * @brief Generates a monotonic ID of the specified type, which is guaranteed to be unique and increasing across calls
		 * @tparam type The type of the monotonic ID to generate, must be constructible from uint64_t
		 */
		template<typename type = uint64_t, typename = std::enable_if_t<std::is_constructible_v<type, uint64_t>>>
		static uint64_t get_mono()
		{
			std::scoped_lock lock{ mutex_ };
			auto timestamp = SDL_GetTicks64();
			static std::atomic<uint64_t> counter{ 1 };
			return type{ (timestamp << 32) | (counter++ & 0xFFFFFFFF) };
		}
	};

	inline std::random_device random::rd{};
	inline std::mt19937_64 random::gen = std::mt19937_64(random::rd());
	inline std::mutex random::mutex_;
}
