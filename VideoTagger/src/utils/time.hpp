#pragma once
#include <chrono>
#include <string>
#include <utils/json.hpp>

namespace vt::utils::time
{
	int64_t diff(std::time_t end, std::time_t start);
	std::string interval_str(int64_t interval);
	std::string utc_timestamp();
}
