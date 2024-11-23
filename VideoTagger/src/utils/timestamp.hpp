#pragma once

#include <chrono>

namespace vt
{
	struct timestamp
	{
		std::chrono::milliseconds total_milliseconds;

		constexpr timestamp() : total_milliseconds{} {}
		constexpr explicit timestamp(std::chrono::milliseconds total_milliseconds) : total_milliseconds{ total_milliseconds } {}
		constexpr explicit timestamp(int64_t milliseconds) : timestamp(std::chrono::milliseconds(milliseconds)) {}

		constexpr timestamp(int64_t hours, int64_t minutes, int64_t seconds, int64_t milliseconds) :
			total_milliseconds{ std::chrono::milliseconds(milliseconds) + std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours) } {}

		constexpr void set(int64_t hours, int64_t minutes, int64_t seconds, int64_t milliseconds)
		{
			milliseconds %= 1000;
			seconds %= 60;
			minutes %= 60;
			total_milliseconds = std::chrono::milliseconds(milliseconds) + std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours);
		}

		constexpr void set_hours(int64_t value)
		{
			total_milliseconds = total_milliseconds - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		constexpr void set_minutes(int64_t value)
		{
			value %= 60;
			total_milliseconds = total_milliseconds - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		constexpr void set_seconds(int64_t value)
		{
			value %= 60;
			total_milliseconds = total_milliseconds - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		constexpr void set_milliseconds(int64_t value)
		{
			value %= 1000;
			total_milliseconds = total_milliseconds - std::chrono::milliseconds(milliseconds()) + std::chrono::milliseconds(value);
		}

		[[nodiscard]] constexpr int64_t hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(total_milliseconds).count();
		}

		[[nodiscard]] constexpr int64_t minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(total_milliseconds).count() % 60;
		}

		[[nodiscard]] constexpr int64_t seconds() const
		{
			return std::chrono::duration_cast<std::chrono::seconds>(total_milliseconds).count() % 60;
		}

		[[nodiscard]] constexpr int64_t milliseconds() const
		{
			return total_milliseconds.count() % 1000;
		}

		static constexpr timestamp zero()
		{
			return timestamp(0);
		}

		constexpr timestamp operator+(const timestamp& other) const
		{
			return timestamp(total_milliseconds + other.total_milliseconds);
		}

		constexpr timestamp& operator+=(const timestamp& other)
		{
			total_milliseconds += other.total_milliseconds;
			return *this;
		}

		constexpr timestamp operator-(const timestamp& other) const
		{
			return timestamp(total_milliseconds - other.total_milliseconds);
		}

		constexpr timestamp& operator-=(const timestamp& other)
		{
			total_milliseconds -= other.total_milliseconds;
			return *this;
		}

		constexpr bool operator==(const timestamp& rhs) const
		{
			return total_milliseconds == rhs.total_milliseconds;
		}

		constexpr bool operator!=(const timestamp& rhs) const
		{
			return !(*this == rhs);
		}

		constexpr bool operator<(const timestamp& rhs) const
		{
			return total_milliseconds < rhs.total_milliseconds;
		}

		constexpr bool operator<=(const timestamp& rhs) const
		{
			return total_milliseconds <= rhs.total_milliseconds;
		}

		constexpr bool operator>(const timestamp& rhs) const
		{
			return total_milliseconds > rhs.total_milliseconds;
		}

		constexpr bool operator>=(const timestamp& rhs) const
		{
			return total_milliseconds >= rhs.total_milliseconds;
		}
	};
}
