#include "pch.hpp"
#include "app_context.hpp"
#include <core/debug.hpp>

namespace vt
{
	void app_context::update_current_video_group()
	{
		if (!current_project.has_value() or current_video_group_id == 0)
		{
			displayed_videos.clear();
			return;
		}

		auto group_it = current_project->video_groups.find(current_video_group_id);
		if (group_it == current_project->video_groups.end())
		{
			current_video_group_id = 0;
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

			it = displayed_videos.erase(it);
		}

		for (auto& group_video_info : active_group)
		{
			auto* pool_video_info = video_pool.get(group_video_info.id);
			if (pool_video_info == nullptr)
			{
				if (auto it = displayed_videos.find(group_video_info.id); it != displayed_videos.end())
				{
					it->video = nullptr;
				}
				continue;
			}

			displayed_videos.insert
			(
				group_video_info.id,
				&pool_video_info->video,
				group_video_info.offset,
				pool_video_info->width,
				pool_video_info->height,
				renderer
			);
		}
		
		displayed_videos.update();
	}

	void app_context::reset_current_video_group()
	{
		displayed_videos.clear();
		displayed_videos.update();
		current_video_group_id = 0;
	}

	segment_storage& app_context::get_current_segment_storage()
	{
		//TODO: maybe do something else
		if (!current_project.has_value())
		{
			debug::panic("No open project");
		}
		if (current_video_group_id == 0)
		{
			debug::panic("No current video group");
		}

		return current_project->segments[current_video_group_id];
	}
}
