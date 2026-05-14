#pragma once
#include <SDL.h>
#include <cstdint>
#include <limits>
#include <utils/math.hpp>

namespace vt
{
	enum class taskbar_state
	{
		none,
		normal,
		indeterminate,
		paused,
		error,
	};

	struct taskbar
	{
	public:
		taskbar(SDL_Window* window);

	private:
		SDL_Window* window_;

	public:
		static void init();
		static void shutdown();

		void set_state(taskbar_state type);
		void reset();

		template<typename type>
		void set_value(type value, type total, type min = std::numeric_limits<type>::min())
		{
			if constexpr (std::is_floating_point_v<type>)
			{
				total = static_cast<type>(1);
				value = math::normalize(value, min, total, static_cast<type>(0), total);
				set_value_impl(static_cast<uint64_t>(value * 10000), static_cast<uint64_t>(total * 10000));
			}
			else
			{
				constexpr auto max_val = std::numeric_limits<uint64_t>::max();
				auto val = math::normalize(value, min, total, static_cast<uint64_t>(0), max_val);
				set_value_impl(val, max_val);
			}
		}

	private:
		void set_value_impl(uint64_t value, uint64_t total);
	};
}
