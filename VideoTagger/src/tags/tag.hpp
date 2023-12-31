#pragma once
#include <string>

namespace vt
{
	struct tag
	{
		tag(std::string name, uint32_t color)
			: name{ name }, color{ color }
		{}

		std::string name;
		//ABGR
		uint32_t color;
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
