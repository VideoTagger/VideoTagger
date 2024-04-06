#pragma once
#include <string>
#include <cstdint>

namespace vt::utils::color
{
	extern std::string to_string(uint32_t color, bool ignore_alpha = true);
	extern bool parse_string(const std::string& string, uint32_t& result, bool overwrite_alpha = true);
}
