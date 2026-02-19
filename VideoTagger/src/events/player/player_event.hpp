#pragma once
#include <events/event.hpp>
#include <widgets/video_player.hpp>

namespace vt
{
	///@brief Base class for all video player related events
	struct player_event : public event
	{
	public:
		constexpr player_event(widgets::video_player& player) : player_{ player } {}

	private:
		widgets::video_player& player_;

	public:
		///@return Reference to the video player associated with this event
		constexpr widgets::video_player& player() const
		{
			return player_;
		}
	};
}
