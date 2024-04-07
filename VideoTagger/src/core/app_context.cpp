#include "pch.hpp"
#include "app_context.hpp"

namespace vt
{
	void app_context::update_active_video_group()
	{
		if (!current_project.has_value() or current_video_group_id == 0)
		{
			videos_manager.clear();
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

		for (auto& group_video_info : active_group)
		{
			auto* pool_video_info = video_pool.get(group_video_info.id);
			if (pool_video_info == nullptr)
			{
				if (auto it = videos_manager.find(group_video_info.id); it != videos_manager.end())
				{
					it->video = nullptr;
				}
				continue;
			}

			videos_manager.insert(
				group_video_info.id,
				&pool_video_info->video,
				group_video_info.offset,
				pool_video_info->width,
				pool_video_info->height,
				renderer
			);
		}
		
		videos_manager.update();
	}

	void app_context::reset_active_video_group()
	{
		videos_manager.clear();
		videos_manager.update();
		current_video_group_id = 0;
	}
}
