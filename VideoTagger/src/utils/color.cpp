#include "pch.hpp"
#include "color.hpp"
#include "string.hpp"

namespace vt::utils::color
{
	std::string to_string(uint32_t color, bool ignore_alpha)
	{
		decltype(color) rgba{};
		if (ignore_alpha)
		{
			//RGB
			rgba = ((color & 0x000000FF) << 16) + (color & 0x0000FF00) + ((color & 0x00FF0000) >> 16);
		}
		else
		{
			//RGBA
			rgba = ((color & 0x000000FF) << 24) + ((color & 0x0000FF00) << 8) + ((color & 0xFF000000) >> 24) + ((color & 0x00FF0000) >> 8);
		}
		return "#" + utils::string::to_hex(rgba, (sizeof(decltype(rgba)) << 1) - (ignore_alpha ? 2 : 0));
	}

	bool parse_string(const std::string& string, uint32_t& result, bool overwrite_alpha)
	{
		size_t size = string.size();
		if (size != 7 and size != 9 or string[0] != '#') return false;

		auto rgba = std::stoul(string.substr(1), nullptr, 16);
		uint32_t r{};
		uint32_t g{};
		uint32_t b{};
		uint32_t a = 0xFF;
		if (size == 9)
		{
			if (!overwrite_alpha)
			{
				a = rgba & 0x000000FF;
			}
			r = (rgba & 0xFF000000) >> 24;
			g = (rgba & 0x00FF0000) >> 16;
			b = (rgba & 0x0000FF00) >> 8;
		}
		else
		{
			r = (rgba & 0xFF0000) >> 16;
			g = (rgba & 0x00FF00) >> 8;
			b = (rgba & 0x0000FF);
		}
		result = r | (g << 8) | (b << 16) | (a << 24); //ABGR
		return true;
	}
}
