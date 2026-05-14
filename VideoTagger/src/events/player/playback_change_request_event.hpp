#pragma once
#include "player_event.hpp"

namespace vt
{
	struct playback_change_request_event : public player_event
	{
	public:
		constexpr playback_change_request_event(widgets::video_player& player, bool is_playing) : player_event(player), is_playing_{ is_playing } {}

	private:
		bool is_playing_;

	public:
		///@return True if the video player is supposed to start playing, false if it is supposed to be paused
		constexpr bool is_playing() const
		{
			return is_playing_;
		}
	};
}
