#pragma once
#include <algorithm>
#include <chrono>
#include <charconv>
#include <nlohmann/json.hpp>

namespace vt
{
	inline constexpr auto full_time_format = "%02u:%02u:%02u:%03u:%03u:%03u";
	inline constexpr auto default_time_format = "%02u:%02u:%02u:%03u";
	inline constexpr auto no_ms_time_format = "%02u:%02u:%02u";

	struct timestamp
	{
		std::chrono::nanoseconds total_nanoseconds{};

		constexpr timestamp() = default;
		constexpr explicit timestamp(std::chrono::nanoseconds total_nanoseconds) : total_nanoseconds{ total_nanoseconds } {}
		constexpr explicit timestamp(int64_t total_nanoseconds) : total_nanoseconds{ total_nanoseconds } {}

		constexpr timestamp(int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanoseconds) : total_nanoseconds
		{
			std::chrono::nanoseconds(std::clamp(nanoseconds, 0, 999)) +
			std::chrono::microseconds(std::clamp(microseconds, 0, 999 )) +
			std::chrono::milliseconds(std::clamp(milliseconds, 0, 999 )) +
			std::chrono::seconds(std::clamp(seconds, 0, 59 )) +
			std::chrono::minutes(std::clamp(minutes, 0, 59 )) +
			std::chrono::hours(hours)
		} {}

		constexpr void set(int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanoseconds)
		{
			total_nanoseconds =
				std::chrono::nanoseconds(std::clamp(nanoseconds, 0, 999)) +
				std::chrono::microseconds(std::clamp(microseconds, 0, 999)) +
				std::chrono::milliseconds(std::clamp(milliseconds, 0, 999)) +
				std::chrono::seconds(std::clamp(seconds, 0, 59)) +
				std::chrono::minutes(std::clamp(minutes, 0, 59)) +
				std::chrono::hours(hours);
		}

		constexpr void set_hours(int value)
		{
			total_nanoseconds = total_nanoseconds - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		constexpr void set_minutes(int value)
		{
			value = std::clamp(value, 0, 59);
			total_nanoseconds = total_nanoseconds - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		constexpr void set_seconds(int value)
		{
			value = std::clamp(value, 0, 59);
			total_nanoseconds = total_nanoseconds - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		constexpr void set_milliseconds(int value)
		{
			value = std::clamp(value, 0, 999);
			total_nanoseconds = total_nanoseconds - std::chrono::milliseconds(milliseconds()) + std::chrono::milliseconds(value);
		}

		constexpr void set_microseconds(int value)
		{
			value = std::clamp(value, 0, 999);
			total_nanoseconds = total_nanoseconds - std::chrono::microseconds(microseconds()) + std::chrono::microseconds(value);
		}

		constexpr void set_nanoseconds(int value)
		{
			value = std::clamp(value, 0, 999);
			total_nanoseconds = total_nanoseconds - std::chrono::nanoseconds(nanoseconds()) + std::chrono::nanoseconds(value);
		}

		[[nodiscard]] constexpr int hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(total_nanoseconds).count();
		}

		[[nodiscard]] constexpr int minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(total_nanoseconds).count() % 60;
		}

		[[nodiscard]] constexpr int seconds() const
		{
			return std::chrono::duration_cast<std::chrono::seconds>(total_nanoseconds).count() % 60;
		}

		[[nodiscard]] constexpr int milliseconds() const
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(total_nanoseconds).count() % 1000;
		}

		[[nodiscard]] constexpr int microseconds() const
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(total_nanoseconds).count() % 1000;
		}

		[[nodiscard]] constexpr int nanoseconds() const
		{
			return total_nanoseconds.count() % 1000;
		}

		static constexpr timestamp zero()
		{
			return timestamp{};
		}

		static constexpr timestamp min()
		{
			return timestamp(std::chrono::nanoseconds::min());
		}

		static constexpr timestamp max()
		{
			return timestamp(std::chrono::nanoseconds::max());
		}

		constexpr timestamp operator+(const timestamp& other) const
		{
			return timestamp(total_nanoseconds + other.total_nanoseconds);
		}

		constexpr timestamp& operator+=(const timestamp& other)
		{
			total_nanoseconds += other.total_nanoseconds;
			return *this;
		}

		constexpr timestamp operator-(const timestamp& other) const
		{
			return timestamp(total_nanoseconds - other.total_nanoseconds);
		}

		constexpr timestamp& operator-=(const timestamp& other)
		{
			total_nanoseconds -= other.total_nanoseconds;
			return *this;
		}

		template<typename number_type>
		constexpr timestamp operator*(number_type scalar) const
		{
			return timestamp(total_nanoseconds * scalar);
		}

		template<typename number_type>
		constexpr timestamp& operator*=(number_type scalar)
		{
			total_nanoseconds *= scalar;
			return *this;
		}

		template<typename number_type>
		constexpr timestamp operator/(number_type scalar) const
		{
			return timestamp(total_nanoseconds / scalar);
		}

		template<typename number_type>
		constexpr timestamp& operator/=(number_type scalar)
		{
			total_nanoseconds /= scalar;
			return *this;
		}

		constexpr bool operator==(const timestamp& other) const
		{
			return total_nanoseconds == other.total_nanoseconds;
		}

		constexpr bool operator!=(const timestamp& other) const
		{
			return !(*this == other);
		}

		constexpr bool operator<(const timestamp& other) const
		{
			return total_nanoseconds < other.total_nanoseconds;
		}

		constexpr bool operator<=(const timestamp& other) const
		{
			return total_nanoseconds <= other.total_nanoseconds;
		}

		constexpr bool operator>(const timestamp& other) const
		{
			return total_nanoseconds > other.total_nanoseconds;
		}

		constexpr bool operator>=(const timestamp& other) const
		{
			return total_nanoseconds >= other.total_nanoseconds;
		}
	};

	inline std::string timestamp_to_string(timestamp ts, const char* format)
	{
		char buffer[256];
		ImFormatString(buffer, IM_ARRAYSIZE(buffer), format, ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds(), ts.microseconds(), ts.nanoseconds());
		return buffer;
	}

	inline constexpr std::optional<timestamp> parse_timestamp(std::string_view input, char separator = ':')
	{
		constexpr int8_t base_60_segments = 2;
		constexpr int8_t base_10_segments = 3;
		constexpr int8_t segment_max = 1 + base_60_segments + base_10_segments;
		timestamp result;

		auto current_ptr = input.data();
		auto end_ptr = input.data() + input.size();
		int8_t current_segment = 0;
		for (auto current_ptr = input.data(); current_ptr != end_ptr and current_segment < segment_max; ++current_segment)
		{
			auto separator_ptr = std::find(current_ptr, end_ptr, separator);

			int64_t segment_value{};
			auto chars_result = std::from_chars(current_ptr, separator_ptr, segment_value);
			if (chars_result.ec != std::errc{} or chars_result.ptr != separator_ptr)
			{
				return std::nullopt;
			}

			current_ptr = separator_ptr;
			if (current_ptr != end_ptr) ++current_ptr;

			int64_t multiplier = 1;
			for (int8_t i = 0; i < base_60_segments - current_segment; i++)
			{
				multiplier *= 60;
			}
			for (int8_t i = 0; i < std::min(base_10_segments, int8_t{ segment_max - 1 - current_segment }); i++)
			{
				multiplier *= 1000;
			}

			result.total_nanoseconds += std::chrono::nanoseconds{ segment_value * multiplier };
		}

		return result;
	}

	inline void to_json(nlohmann::ordered_json& json, const timestamp& ts)
	{
		json = timestamp_to_string(ts, full_time_format);
	}

	inline void from_json(const nlohmann::ordered_json& json, timestamp& ts)
	{
		auto ts_opt = parse_timestamp(json);
		if (!ts_opt.has_value()) return;

		ts = *ts_opt;
	}
}
