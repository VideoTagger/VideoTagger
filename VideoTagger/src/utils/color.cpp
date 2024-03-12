#include "color.hpp"
#include "string.hpp"

namespace vt::utils::color
{
	std::string to_string(uint32_t color)
	{
		decltype(color) rgb = ((color & 0x0000FF) << 16) + (color & 0x00FF00) + ((color & 0xFF0000) >> 16);
		return "#" + utils::string::to_hex(rgb, (sizeof(decltype(rgb)) << 1) - 2);
	}

	bool parse_string(const std::string& string, uint32_t& result)
	{
		if (string.size() != 7 or string[0] != '#') return false;

		auto rgb = std::stoul(string.substr(1), nullptr, 16);
		result = (0xFF << 24) + ((rgb & 0x0000FF) << 16) + (rgb & 0x00FF00) + ((rgb & 0xFF0000) >> 16); //ABGR
		return true;
	}
}
