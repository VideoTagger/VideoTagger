#pragma once
#include "timestamp.hpp"

namespace vt::utils
{
	struct timestamp_span
	{
		constexpr timestamp_span() = default;
		constexpr timestamp_span(timestamp start, timestamp end) : start{ start }, end{ end } {}

		timestamp start{};
		timestamp end{};

		constexpr int64_t length() const
		{
			return (end - start).total_milliseconds.count();
		}
	};
}
