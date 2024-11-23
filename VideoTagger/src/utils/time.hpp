#pragma once
#include <chrono>
#include <string>
#include "timestamp.hpp"
#include <utils/json.hpp>

namespace vt::utils::time
{
	int64_t diff(std::time_t end, std::time_t start);
	std::string interval_str(int64_t interval);

	constexpr auto default_time_format = "%02u:%02u:%02u:%03u";
	std::string time_to_string(int64_t milliseconds , const char* format = default_time_format);
	int64_t parse_time_to_ms(const std::string& input, char separator = ':');
	std::string utc_timestamp();
}

namespace vt
{
	inline void to_json(nlohmann::ordered_json& json, const timestamp& ts)
	{
		json = utils::time::time_to_string(ts.total_milliseconds.count());
	}

	inline void from_json(const nlohmann::ordered_json& json, timestamp& ts)
	{
		ts.total_milliseconds = std::chrono::milliseconds(utils::time::parse_time_to_ms(json));
	}
}
