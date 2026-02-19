#pragma once
#include "player_event.hpp"

namespace vt
{
	struct seek_backwards_event : public player_event
	{
	public:
		constexpr seek_backwards_event(widgets::video_player& player) : player_event(player) {}
	};
}
