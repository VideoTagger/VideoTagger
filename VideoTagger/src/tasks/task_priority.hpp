#pragma once
#include <cstdint>

namespace vt
{
	enum class task_priority : int8_t
	{
		highest,
		high,
		normal,
		low ,
		lowest,
	};

	static constexpr size_t task_priority_count = 5;
}
