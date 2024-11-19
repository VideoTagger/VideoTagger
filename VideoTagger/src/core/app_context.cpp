#include "pch.hpp"
#include "app_context.hpp"
#include <core/debug.hpp>
#include <services/google/google_account_manager.hpp>
#include <video/local_video_importer.hpp>

namespace vt
{
	void app_context::register_account_managers()
	{
		register_account_manager<google_account_manager>();
	}

	service_account_manager& app_context::get_account_manager(const std::string& service_id)
	{
		return *account_managers.at(service_id);
	}

	bool app_context::is_account_manager_registered(const std::string& service_id) const
	{
		return account_managers.count(service_id) != 0;
	}

	void app_context::register_video_importers()
	{
		register_video_importer<local_video_importer>();
	}

	video_importer& app_context::get_video_importer(const std::string& importer_id)
	{
		return *video_importers.at(importer_id);
	}

	bool app_context::is_video_importer_registered(const std::string& importer_id) const
	{
		return video_importers.count(importer_id) != 0;
	}

	void app_context::register_handlers()
	{

	}

	void app_context::update_current_video_group()
	{
		displayed_videos.update();
	}

	void app_context::reset_current_video_group()
	{
		set_current_video_group_id(invalid_video_group_id);
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
		if (!current_project.has_value())
		{
			return;
		}

		if (id == current_video_group_id_)
		{
			return;
		}

		if (id != invalid_video_group_id and current_project->video_groups.count(id) == 0)
		{
			debug::error("Tried to set video group to id {} which doesn't exist", id);
			return;
		}

		current_video_group_id_ = id;
		video_timeline.moving_segment.reset();
		video_timeline.selected_segment.reset();
		insert_segment_data.clear();

		displayed_videos.clear();
		
		if (id == invalid_video_group_id)
		{
			return;
		}

		for (auto& group_inf : current_project->video_groups.at(id))
		{
			auto& vid_resource = current_project->videos.get(group_inf.id);
			const auto& metadata = vid_resource.metadata();
			if (!vid_resource.available())
			{
				debug::error("Video {} with id {} is not available", metadata.title.has_value() ? *metadata.title : "[UNTITLED]");
				continue;
			}

			displayed_videos.insert(vid_resource.id(), vid_resource.video(), group_inf.offset, *metadata.width, *metadata.height);
		}

		ctx_.reset_player_docking = true;
	}

	video_group_id_t app_context::current_video_group_id() const
	{
		return current_video_group_id_;
	}
}
