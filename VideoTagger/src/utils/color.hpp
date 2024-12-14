#pragma once
#include <string>
#include <cstdint>

namespace vt::utils::color
{
	extern std::string to_string(uint32_t color, bool ignore_alpha = true);
	extern bool parse_string(const std::string& string, uint32_t& result, bool overwrite_alpha = true);
	constexpr inline uint32_t to_abgr(uint32_t rgba)
	{
		uint8_t r = (rgba >> 24) & 0xFF;
		uint8_t g = (rgba >> 16) & 0xFF;
		uint8_t b = (rgba >> 8) & 0xFF;
		uint8_t a = (rgba >> 0) & 0xFF;

		return (a << 24) | (b << 16) | (g << 8) | r;
	}
}
