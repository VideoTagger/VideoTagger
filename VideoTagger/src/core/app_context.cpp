#include "pch.hpp"
#include "app_context.hpp"

namespace vt
{
	void app_context::update_active_video_group()
	{
		group_manager.videos.clear();
		if (!current_project.has_value() or current_video_group_id == 0)
		{
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

			video_group_data video_data = {
				group_video_info.id,
				pool_video_info ? &pool_video_info->video : nullptr,
				group_video_info.offset
			};

			group_manager.videos.push_back(video_data);
		}

		group_manager.update();
	}

	void app_context::reset_active_video_group()
	{
		group_manager.videos.clear();
		group_manager.update();
		current_video_group_id = 0;
	}
}
