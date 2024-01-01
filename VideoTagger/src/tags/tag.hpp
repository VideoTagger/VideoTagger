#pragma once
#include <string>
#include <cstdint>

#include "tag_timeline.hpp"

namespace vt
{
	struct tag
	{
		std::string name;
		//ABGR
		uint32_t color{};
		tag_timeline timeline;

		tag(std::string name, uint32_t color)
			: name{ name }, color{ color | 0xff000000 }
		{
		}
	};

	inline bool operator==(const tag& lhs, const tag& rhs)
	{
		return lhs.name == rhs.name;
	}

	inline bool operator!=(const tag& lhs, const tag& rhs)
	{
		return !(lhs == rhs);
	}
}

template <>
struct std::hash<vt::tag>
{
	std::size_t operator()(const vt::tag& value) const
	{
		return std::hash<std::string>{}(value.name);
	}
};
