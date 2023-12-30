#pragma once

#include <chrono>

namespace vt
{
	//come up with a better name
	struct video_time_t
	{
		std::chrono::seconds total_seconds;

		constexpr video_time_t() : total_seconds{} {}

		constexpr explicit video_time_t(std::chrono::seconds total_seconds) : total_seconds{ total_seconds } {}

		constexpr video_time_t(uint16_t hours, uint8_t minutes, uint8_t seconds) :
			total_seconds{ std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours) } {}

		constexpr void set(uint16_t hours, uint8_t minutes, uint8_t seconds)
		{
			seconds %= 60;
			minutes %= 60;
			total_seconds = std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours);
		}

		constexpr void set_seconds(uint8_t value)
		{
			value %= 60;
			total_seconds = total_seconds - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		constexpr void set_minutes(uint8_t value)
		{
			value %= 60;
			total_seconds = total_seconds - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		constexpr void set_hours(uint16_t value)
		{
			total_seconds = total_seconds - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		[[nodiscard]] constexpr uint16_t hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(total_seconds).count();
		}

		[[nodiscard]] constexpr uint8_t minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(total_seconds).count() % 60;
		}

		[[nodiscard]] constexpr uint8_t seconds() const
		{
			return total_seconds.count() % 60;
		}
	};
}
