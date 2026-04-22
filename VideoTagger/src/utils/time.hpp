#pragma once
#include <chrono>
#include <string>
#include <utils/json.hpp>

namespace vt::utils::time
{
	int64_t diff(std::time_t end, std::time_t start);
	std::string interval_str(int64_t interval);

	constexpr auto default_time_format = "%02u:%02u:%02u:%03u";
	constexpr auto no_ms_time_format = "%02u:%02u:%02u";

	std::string time_to_string(int64_t milliseconds, const char* format = default_time_format);
	int64_t parse_time_to_ms(const std::string& input, char separator = ':');
	std::string utc_timestamp();
}
