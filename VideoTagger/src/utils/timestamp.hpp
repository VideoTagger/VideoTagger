#pragma once

#include <chrono>

namespace vt
{
	struct timestamp
	{
		std::chrono::seconds seconds_total;

		constexpr timestamp() : seconds_total{} {}
		constexpr explicit timestamp(std::chrono::seconds seconds_total) : seconds_total{ seconds_total } {}
		constexpr explicit timestamp(uint64_t seconds) : timestamp(std::chrono::seconds(seconds)) {}

		constexpr timestamp(uint32_t hours, uint16_t minutes, uint16_t seconds) : seconds_total{ std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours) } {}

		constexpr void set(uint32_t hours, uint16_t minutes, uint16_t seconds)
		{
			seconds %= 60;
			minutes %= 60;
			seconds_total = std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours);
		}

		constexpr void set_hours(uint64_t value)
		{
			seconds_total = seconds_total - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		constexpr void set_minutes(uint64_t value)
		{
			value %= 60;
			seconds_total = seconds_total - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		constexpr void set_seconds(uint64_t value)
		{
			value %= 60;
			seconds_total = seconds_total - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		[[nodiscard]] constexpr uint64_t hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(seconds_total).count();
		}

		[[nodiscard]] constexpr uint64_t minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(seconds_total).count() % 60;
		}

		[[nodiscard]] constexpr uint64_t seconds() const
		{
			return seconds_total.count() % 60;
		}

		static constexpr timestamp zero()
		{
			return timestamp(0);
		}

		constexpr timestamp operator+(const timestamp& other) const
		{
			return timestamp(seconds_total + other.seconds_total);
		}

		constexpr timestamp& operator+=(const timestamp& other)
		{
			seconds_total += other.seconds_total;
			return *this;
		}

		constexpr timestamp operator-(const timestamp& other) const
		{
			return timestamp(seconds_total - other.seconds_total);
		}

		constexpr timestamp& operator-=(const timestamp& other)
		{
			seconds_total -= other.seconds_total;
			return *this;
		}

		constexpr bool operator==(const timestamp& rhs) const
		{
			return seconds_total == rhs.seconds_total;
		}

		constexpr bool operator!=(const timestamp& rhs) const
		{
			return !(*this == rhs);
		}

		constexpr bool operator<(const timestamp& rhs) const
		{
			return seconds_total < rhs.seconds_total;
		}

		constexpr bool operator<=(const timestamp& rhs) const
		{
			return seconds_total <= rhs.seconds_total;
		}

		constexpr bool operator>(const timestamp& rhs) const
		{
			return seconds_total > rhs.seconds_total;
		}

		constexpr bool operator>=(const timestamp& rhs) const
		{
			return seconds_total >= rhs.seconds_total;
		}
	};
}
