#pragma once
#include <string>
#include <cstdint>

namespace vt::utils::color
{
	extern std::string to_string(uint32_t color);
	extern bool parse_string(const std::string& string, uint32_t& result);
}
