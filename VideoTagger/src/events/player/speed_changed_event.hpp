#pragma once
#include "player_event.hpp"

namespace vt
{
	struct speed_changed_event : public player_event
	{
	public:
		constexpr speed_changed_event(widgets::video_player& player, float speed) : player_event(player), speed_{ speed } {}

	private:
		float speed_;

	public:
		///@return Speed of the video player playback
		constexpr float speed() const
		{
			return speed_;
		}
	};
}
