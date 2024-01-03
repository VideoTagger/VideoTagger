#pragma once
#include <chrono>
#include <string>
#include "timestamp.hpp"

namespace vt::utils::time
{
	timestamp diff(std::tm& left, std::tm& right);
	std::string duration_str(timestamp duration);
}
