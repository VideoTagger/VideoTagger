#pragma once
#include <chrono>
#include "player_event.hpp"

namespace vt
{
	struct seek_event : public player_event
	{
	public:
		constexpr seek_event(widgets::video_player& player, std::chrono::nanoseconds ts) : player_event(player), ts_{ ts } {}

	private:
		std::chrono::nanoseconds ts_;

	public:
		///@return Target timestamp of the seek event
		constexpr std::chrono::nanoseconds timestamp() const
		{
			return ts_;
		}
	};
}
