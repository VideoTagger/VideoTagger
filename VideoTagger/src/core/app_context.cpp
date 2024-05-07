#include "pch.hpp"
#include "app_context.hpp"
#include <core/debug.hpp>

namespace vt
{
	void app_context::update_current_video_group()
	{
		//TODO: needs a refactor

		auto group_it = current_project->video_groups.find(current_video_group_id_);
		if (!current_project.has_value() or current_video_group_id_ == invalid_video_group_id or group_it == current_project->video_groups.end())
		{
			set_current_video_group_id(invalid_video_group_id);
			auto& video_pool = current_project->videos;

			for (auto it = displayed_videos.begin(); it != displayed_videos.end();)
			{
				if (auto metadata = video_pool.get(it->id); metadata != nullptr)
				{
					metadata->is_widget_open = false;
					//TODO: should it close?
					metadata->close_video();
				}

				it = displayed_videos.erase(it);
			}

			displayed_videos.clear();
			return;
		}

		auto& active_group = group_it->second;
		auto& video_pool = current_project->videos;

		for (auto it = displayed_videos.begin(); it != displayed_videos.end();)
		{
			if (active_group.contains(it->id))
			{
				it++;
				continue;
			}

			if (auto metadata = video_pool.get(it->id); metadata != nullptr)
			{
				metadata->is_widget_open = false;
				//TODO: should it close?
				metadata->close_video();
			}

			it = displayed_videos.erase(it);
		}

		for (auto& group_video_info : active_group)
		{
			auto* pool_video_metadata = video_pool.get(group_video_info.id);
			if (pool_video_metadata == nullptr)
			{
				if (auto it = displayed_videos.find(group_video_info.id); it != displayed_videos.end())
				{
					it->video = nullptr;
				}
				continue;
			}

			//TODO: maybe come up with some better way to do this
			if (!pool_video_metadata->is_widget_open)
			{
				pool_video_metadata->is_widget_open = true;
				ctx_.reset_player_docking = true;
			}
			if (!pool_video_metadata->video.is_open())
			{
				pool_video_metadata->open_video();
			}

			displayed_videos.insert
			(
				group_video_info.id,
				&pool_video_metadata->video,
				group_video_info.offset,
				pool_video_metadata->width,
				pool_video_metadata->height,
				renderer
			);
		}
		
		displayed_videos.update();
	}

	void app_context::reset_current_video_group()
	{
		set_current_video_group_id(invalid_video_group_id);
		update_current_video_group();
	}

	segment_storage& app_context::get_current_segment_storage()
	{
		//TODO: maybe do something else
		if (!current_project.has_value())
		{
			debug::panic("No open project");
		}
		if (current_video_group_id_ == invalid_video_group_id)
		{
			debug::panic("No current video group");
		}

		return current_project->segments[current_video_group_id_];
	}

	void app_context::set_current_video_group_id(video_group_id_t id)
	{
		if (id == current_video_group_id_)
		{
			return;
		}

		current_video_group_id_ = id;
		moving_segment_data.reset();
		selected_segment_data.reset();
		insert_segment_data.clear();
	}

	video_group_id_t app_context::current_video_group_id() const
	{
		return current_video_group_id_;
	}
}
