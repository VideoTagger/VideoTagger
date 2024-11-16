#pragma once
#include <nlohmann/json.hpp>

namespace vt
{
	struct account_info
	{
		bool active{};
		nlohmann::json properties;
	};
}
