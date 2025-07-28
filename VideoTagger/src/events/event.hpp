#pragma once

namespace vt
{
	///@brief Base class for all events
	struct event
	{
		constexpr event() = default;
		constexpr event(const event&) = delete;
	};
}
