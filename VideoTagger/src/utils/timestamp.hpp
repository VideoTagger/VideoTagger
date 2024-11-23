#pragma once

#include <chrono>

namespace vt
{
	struct timestamp
	{
		std::chrono::seconds seconds_total;

		constexpr timestamp() : seconds_total{} {}
		constexpr explicit timestamp(std::chrono::seconds seconds_total) : seconds_total{ seconds_total } {}
		constexpr explicit timestamp(int64_t seconds) : timestamp(std::chrono::seconds(seconds)) {}

		constexpr timestamp(int64_t hours, int64_t minutes, int64_t seconds) : seconds_total{ std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours) } {}

		constexpr void set(int64_t hours, int64_t minutes, int64_t seconds)
		{
			seconds %= 60;
			minutes %= 60;
			seconds_total = std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours);
		}

		constexpr void set_hours(int64_t value)
		{
			seconds_total = seconds_total - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		constexpr void set_minutes(int64_t value)
		{
			value %= 60;
			seconds_total = seconds_total - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		constexpr void set_seconds(int64_t value)
		{
			value %= 60;
			seconds_total = seconds_total - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		[[nodiscard]] constexpr int64_t hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(seconds_total).count();
		}

		[[nodiscard]] constexpr int64_t minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(seconds_total).count() % 60;
		}

		[[nodiscard]] constexpr int64_t seconds() const
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
