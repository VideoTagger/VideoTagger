#pragma once
#include "player_event.hpp"

namespace vt
{
	struct looping_changed_event : public player_event
	{
	public:
		constexpr looping_changed_event(widgets::video_player& player, loop_mode mode) : player_event(player), mode_{ mode } {}

	private:
		loop_mode mode_;

	public:
		///@return Loop mode of the video player
		constexpr loop_mode mode() const
		{
			return mode_;
		}
	};
}
