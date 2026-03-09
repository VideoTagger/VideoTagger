#pragma once
#include "player_event.hpp"
#include <video/video_pool.hpp>

namespace vt
{
	class playback_reached_end_event : public player_event
	{
	public:
		playback_reached_end_event(widgets::video_player& player, video_group_id_t group_id) :
			player_event{ player }, group_id_{ group_id } {}

	private:
		video_group_id_t group_id_;
		bool playback_ended_;

	public:
		constexpr video_group_id_t group_id() const
		{
			return group_id_;
		}
	};
}
