#pragma once
#include <chrono>
#include <string>
#include "timestamp.hpp"

namespace vt::utils::time
{
	uint64_t diff(std::time_t end, std::time_t start);
	std::string interval_str(uint64_t interval);

	constexpr auto default_time_format = "%02u:%02u:%02u";
	extern std::string time_to_string(uint64_t seconds, const char* format = default_time_format);
	extern uint64_t parse_time_to_sec(const std::string& input, char separator = ':');
}
