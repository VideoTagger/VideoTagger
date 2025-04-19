#include "pch.hpp"
#include "app_context.hpp"
#include <core/debug.hpp>
#include <utils/filesystem.hpp>
#include <services/google/google_account_manager.hpp>
#include <video/local_video_importer.hpp>
#include <video/google_drive/google_drive_video_importer.hpp>

#include <editor/run_script_command.hpp>
#include <editor/selected_attribute_query.hpp>
#include <editor/set_selected_attribute_command.hpp>
#include <editor/active_video_tex_size_query.hpp>

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
		register_video_importer<google_drive_video_importer>();
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
		registry.register_command_handler<run_script_command_handler>();
		registry.register_command_handler<set_selected_attribute_command_handler>();
		
		registry.register_query_handler<selected_attribute_query_handler>();
		registry.register_query_handler<active_video_tex_size_query_handler>();
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

		return current_project->video_groups.at(current_video_group_id_).segments();
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
			if (!vid_resource.playable())
			{
				debug::error("Video {} with id {} is not available", metadata.title.has_value() ? *metadata.title : "[UNTITLED]", vid_resource.id());
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

	std::shared_ptr<lang_pack> app_context::load_lang_pack(const std::string& name)
	{
		auto path = lang_dir_filepath / (name + "." + lang_pack::extension);
		debug::log("Loading lang pack with name: '{}' from path: '{}'", name, path.u8string());
		if (!std::filesystem::exists(path))
		{
			debug::error("Lang pack with name: '{}' not found", name);
			return nullptr;
		}
		auto new_lang = lang_pack::load_from_file(path);
		if (!new_lang.has_value()) return nullptr;
		return std::make_shared<lang_pack>(new_lang.value());
	}

	std::shared_ptr<lang_pack> app_context::load_or_create_lang_pack(const std::string& name, const std::string& filename)
	{
		auto path = lang_dir_filepath / (filename + "." + lang_pack::extension);
		debug::log("Loading lang pack with name: '{}' from path: '{}'", name, path.u8string());
		if (!std::filesystem::exists(path))
		{
			debug::error("Lang pack with name: '{}' not found, creating new lang pack...", name);
			return std::make_shared<lang_pack>(name, filename);
		}
		auto new_lang = lang_pack::load_from_file(path);
		if (!new_lang.has_value()) return nullptr;
		return std::make_shared<lang_pack>(new_lang.value());
	}

    void app_context::instert_lang_pack(std::shared_ptr<lang_pack> pack)
    {
		lang_packs.push_back(pack);
    }

	void app_context::load_lang_packs(const std::string& desired_lang)
	{
		ctx_.lang_packs.clear();
		for (const auto& entry : std::filesystem::directory_iterator{ ctx_.lang_dir_filepath })
		{
			auto path = entry.path();
			if (entry.is_directory() or path.extension() != std::string(".") + lang_pack::extension) continue;
			auto lang = lang_pack::load_from_file(path);
			if (!lang.has_value()) continue;
			ctx_.lang_packs.push_back(std::make_shared<lang_pack>(lang.value()));
		}

		if (ctx_.lang_packs.empty())
		{
			debug::error("No lang packs found, creating default lang pack...");
			auto lang = std::make_shared<lang_pack>("English", "en_US");
			ctx_.lang_packs.push_back(lang);
		}

		auto it = std::find_if(ctx_.lang_packs.begin(), ctx_.lang_packs.end(), [&](const auto& lang)
		{
			return lang->filename() == desired_lang;
		});

		if (it != ctx_.lang_packs.end())
		{
			ctx_.lang = *it;
		}
		else
		{
			ctx_.lang = ctx_.lang_packs.front();
		}
	}

    std::filesystem::path app_context::storage_path()
    {
        return utils::filesystem::get_storage_path("VideoTagger", "VideoTagger");
    }
}
