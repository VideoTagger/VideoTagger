#pragma once
#include "player_event.hpp"

namespace vt
{
	struct playback_changed_event : public player_event
	{
	public:
		constexpr playback_changed_event(widgets::video_player& player, bool is_playing) : player_event(player), is_playing_{ is_playing } {}

	private:
		bool is_playing_;

	public:
		///@return True if the video player is playing, false if it is paused
		constexpr bool is_playing() const
		{
			return is_playing_;
		}
	};
}
