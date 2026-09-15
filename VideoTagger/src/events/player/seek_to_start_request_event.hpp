#pragma once
#include "player_event.hpp"

namespace vt
{
	struct seek_to_start_request_event : public player_event
	{
	public:
		constexpr seek_to_start_request_event(widgets::video_player& player) : player_event(player) {}
	};
}
