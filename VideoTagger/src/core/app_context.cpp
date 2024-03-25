#include "app_context.hpp"

namespace vt
{
	void app_context::update_active_video_group()
	{
		active_video_group.videos.clear();
		if (!current_project.has_value() or active_video_group_id == 0)
		{
			return;
		}

		auto group_it = current_project->video_groups.find(active_video_group_id);
		if (group_it == current_project->video_groups.end())
		{
			active_video_group_id = 0;
			return;
		}

		auto& active_group = group_it->second;
		auto& video_pool = current_project->videos;

		for (auto& group_video_info : active_group)
		{
			auto* pool_video_info = video_pool.get(group_video_info.id);

			active_video_data video_data = {
				group_video_info.id,
				pool_video_info ? &pool_video_info->video : nullptr,
				group_video_info.offset
			};

			active_video_group.videos.push_back(video_data);
		}

		active_video_group.update();
	}

	void app_context::reset_active_video_group()
	{
		active_video_group.videos.clear();
		active_video_group.update();
		active_video_group_id = 0;
	}
}
