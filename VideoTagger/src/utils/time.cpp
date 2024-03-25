#include "time.hpp"
#include <ctime>
#include <cmath>

#include <imgui_internal.h>

namespace vt::utils::time
{
	uint64_t diff(std::time_t end, std::time_t start)
	{
		return static_cast<uint64_t>(std::abs(std::difftime(end, start)));
	}
	
	std::string interval_str(uint64_t interval)
	{
		//TODO: This most likely could be written better

		static constexpr uint64_t days_in_year = 365;
		static constexpr uint64_t days_in_month = 30;
		static constexpr uint64_t days_in_week = 7;
		static constexpr uint64_t hrs_in_day = 24;
		static constexpr uint64_t mins_in_hour = 60;
		static constexpr uint64_t sec_in_min = 60;

		static constexpr uint64_t sec_in_hour = sec_in_min * mins_in_hour;
		static constexpr uint64_t sec_in_day = sec_in_hour * hrs_in_day;
		static constexpr uint64_t sec_in_week = sec_in_day * days_in_week;
		static constexpr uint64_t sec_in_month = sec_in_day * days_in_month;
		static constexpr uint64_t sec_in_year = sec_in_day * days_in_year;

		uint64_t years = interval / sec_in_year;
		uint64_t months = (interval % sec_in_year) / sec_in_month;
		uint64_t weeks = ((interval % sec_in_year) % sec_in_month) / sec_in_week;
		uint64_t days = (((interval % sec_in_year) % sec_in_month) % sec_in_week) / sec_in_day;
		uint64_t hours = ((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) / sec_in_hour;
		uint64_t minutes = (((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) % sec_in_hour) / sec_in_min;
		uint64_t seconds = ((((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) % sec_in_hour) % sec_in_min);

		if (years > 0) return std::to_string(years) + (years > 1 ? " years" : " year");
		if (months > 0) return std::to_string(months) + (months > 1 ? " months" : " month");
		if (days > 0) return std::to_string(days) + (days > 1 ? " days" : " day");
		if (hours > 0) return std::to_string(hours) + (hours > 1 ? " hours" : " hour");
		if (minutes > 0) return std::to_string(minutes) + (minutes > 1 ? " minutes" : " minute");
		if (seconds > 0) return std::to_string(seconds) + (seconds > 1 ? " seconds" : " second");
		return {};
	}

	std::string time_to_string(uint64_t seconds, const char* format)
	{
		timestamp ts(seconds);
		char buffer[256];
		ImFormatString(buffer, IM_ARRAYSIZE(buffer), format, ts.hours(), ts.minutes(), ts.seconds());
		return buffer;
	}

	//parses HH:MM:SS string into seconds
	uint64_t parse_time_to_sec(const std::string& input, char separator)
	{
		uint8_t n = 0;
		uint8_t segment = 0;
		constexpr uint8_t segment_max = 3;
		uint64_t seconds{};
		uint64_t val{};
		auto it = input.rbegin();
		while (it != input.rend() and segment != segment_max)
		{
			char c = *it++;
			bool is_separator = (c == separator);
			if (!is_separator)
			{
				val += static_cast<uint64_t>(c - '0') * static_cast<uint64_t>(std::pow(10, n++));
			}
			if (is_separator or it == input.rend())
			{
				seconds += val * static_cast<uint64_t>(std::pow(60ull, segment++));
				n = 0;
				val = 0;
			}
		}
		return seconds;
	}
}
