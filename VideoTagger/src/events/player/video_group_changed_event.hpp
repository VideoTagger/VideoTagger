#pragma once
#include "player_event.hpp"
#include <video/video_pool.hpp>

namespace vt
{
	class video_group_changed_event : public player_event
	{
	public:
		video_group_changed_event(widgets::video_player& player, video_group_id_t old_group_id, video_group_id_t new_group_id) :
			player_event{ player }, old_group_id_{ old_group_id }, new_group_id_{new_group_id} { }

	private:
		video_group_id_t old_group_id_;
		video_group_id_t new_group_id_;

	public:
		constexpr video_group_id_t new_group_id() const
		{
			return new_group_id_;
		}

		constexpr video_group_id_t old_group_id() const
		{
			return old_group_id_;
		}
	};
}
