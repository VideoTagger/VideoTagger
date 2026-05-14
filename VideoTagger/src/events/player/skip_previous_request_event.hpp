#pragma once
#include "player_event.hpp"

namespace vt
{
	struct skip_previous_request_event : public player_event
	{
	public:
		constexpr skip_previous_request_event(widgets::video_player& player) : player_event(player) {}
	};
}
