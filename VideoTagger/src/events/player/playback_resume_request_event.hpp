#pragma once
#include "player_event.hpp"

namespace vt
{
	struct playback_resume_request_event : public player_event
	{
		constexpr playback_resume_request_event(widgets::video_player& player) : player_event(player) {}
	};
}
