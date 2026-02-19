#pragma once
#include "player_event.hpp"

namespace vt
{
	struct seek_forwards_event : public player_event
	{
	public:
		constexpr seek_forwards_event(widgets::video_player& player) : player_event(player) {}
	};
}
