#include "pch.hpp"
#include "time.hpp"
#include <iomanip>
#include <sstream>

namespace vt::utils::time
{
	int64_t diff(std::time_t end, std::time_t start)
	{
		return static_cast<int64_t>(std::abs(std::difftime(end, start)));
	}
	
	std::string interval_str(int64_t interval)
	{
		//TODO: This most likely could be written better

		static constexpr int64_t days_in_year = 365;
		static constexpr int64_t days_in_month = 30;
		static constexpr int64_t days_in_week = 7;
		static constexpr int64_t hrs_in_day = 24;
		static constexpr int64_t mins_in_hour = 60;
		static constexpr int64_t sec_in_min = 60;

		static constexpr int64_t sec_in_hour = sec_in_min * mins_in_hour;
		static constexpr int64_t sec_in_day = sec_in_hour * hrs_in_day;
		static constexpr int64_t sec_in_week = sec_in_day * days_in_week;
		static constexpr int64_t sec_in_month = sec_in_day * days_in_month;
		static constexpr int64_t sec_in_year = sec_in_day * days_in_year;

		int64_t years = interval / sec_in_year;
		int64_t months = (interval % sec_in_year) / sec_in_month;
		int64_t weeks = ((interval % sec_in_year) % sec_in_month) / sec_in_week;
		int64_t days = (((interval % sec_in_year) % sec_in_month) % sec_in_week) / sec_in_day;
		int64_t hours = ((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) / sec_in_hour;
		int64_t minutes = (((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) % sec_in_hour) / sec_in_min;
		int64_t seconds = ((((((interval % sec_in_year) % sec_in_month) % sec_in_week) % sec_in_day) % sec_in_hour) % sec_in_min);

		if (years > 0) return std::to_string(years) + (years > 1 ? " years" : " year");
		if (months > 0) return std::to_string(months) + (months > 1 ? " months" : " month");
		if (days > 0) return std::to_string(days) + (days > 1 ? " days" : " day");
		if (hours > 0) return std::to_string(hours) + (hours > 1 ? " hours" : " hour");
		if (minutes > 0) return std::to_string(minutes) + (minutes > 1 ? " minutes" : " minute");
		if (seconds > 0) return std::to_string(seconds) + (seconds > 1 ? " seconds" : " second");
		return {};
	}

	std::string time_to_string(int64_t seconds, const char* format)
	{
		timestamp ts(seconds);
		char buffer[256];
		ImFormatString(buffer, IM_ARRAYSIZE(buffer), format, ts.hours(), ts.minutes(), ts.seconds());
		return buffer;
	}

	//parses HH:MM:SS string into seconds
	int64_t parse_time_to_sec(const std::string& input, char separator)
	{
		uint8_t n = 0;
		uint8_t segment = 0;
		constexpr uint8_t segment_max = 3;
		int64_t seconds{};
		int64_t val{};
		auto it = input.rbegin();
		while (it != input.rend() and segment != segment_max)
		{
			char c = *it++;
			bool is_separator = (c == separator);
			if (!is_separator)
			{
				val += static_cast<int64_t>(c - '0') * static_cast<int64_t>(std::pow(10, n++));
			}
			if (is_separator or it == input.rend())
			{
				seconds += val * static_cast<int64_t>(std::pow(60ull, segment++));
				n = 0;
				val = 0;
			}
		}
		return seconds;
	}

    std::string utc_timestamp()
    {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);

		std::tm utc_tm;
#if defined(_WIN32) or defined(_WIN64)
		gmtime_s(&utc_tm, &now_time);
#else
		gmtime_r(&now_time, &utc_tm);
#endif
		std::ostringstream offset_ss;
		offset_ss << std::put_time(&utc_tm, "%z");
		std::string offset = offset_ss.str();
		if (offset.empty())
		{
			offset = "Z";
		}
		else
		{
			offset = fmt::format("{}:{}", offset.substr(0, 3), offset.substr(3));
		}
		std::ostringstream oss;
		oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S") << offset;
		return oss.str();
    }
}
