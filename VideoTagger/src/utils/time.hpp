#pragma once
#include <chrono>
#include <string>
#include "timestamp.hpp"

namespace vt::utils::time
{
	uint64_t diff(std::time_t end, std::time_t start);
	std::string interval_str(uint64_t interval);
}
