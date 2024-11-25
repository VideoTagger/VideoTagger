#include "pch.hpp"
#include "downloadable_video_resource.hpp"
#include <core/app_context.hpp>

namespace vt
{
	bool video_download_result::is_done() const
	{
		return result.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready;
	}

	void video_download_result::cancel()
	{
		if (data != nullptr)
		{
			data->cancel = true;
		}
	}

	downloadable_video_resource::downloadable_video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata)
		: video_resource(std::move(importer_id), std::move(id), std::move(metadata))
	{
	}

	downloadable_video_resource::downloadable_video_resource(std::string importer_id, const nlohmann::ordered_json& json)
		: video_resource(std::move(importer_id), json)
	{
		if (json.contains("local_path"))
		{
			local_path_ = std::string(json.at("local_path"));
		}
	}

	const std::filesystem::path& downloadable_video_resource::local_path() const
	{
		return local_path_;
	}

	void downloadable_video_resource::set_local_path(std::filesystem::path path)
	{
		local_path_ = std::move(path);
	}

	video_download_result downloadable_video_resource::download(std::launch launch_policy)
	{
		video_download_result result;
		result.data = std::make_shared<video_download_data>();
		result.result = std::async(launch_policy, get_download_function(), result.data);
		return result;
	}

	std::optional<float> downloadable_video_resource::download_progress() const
	{
		auto ptr = download_data_.lock();
		if (ptr == nullptr)
		{
			return std::nullopt;
		}

		return ptr->progress;
	}

	void downloadable_video_resource::remove_downloaded()
	{
		if (!available())
		{
			return;
		}

		std::filesystem::remove(local_path_);
		local_path_.clear();
	}

	bool downloadable_video_resource::available() const
	{
		return std::filesystem::is_regular_file(local_path_);
	}

	void downloadable_video_resource::on_save(nlohmann::ordered_json& json) const
	{
		json["local_path"] = local_path_.u8string();
	}

	void downloadable_video_resource::on_remove()
	{
		remove_downloaded();
	}

	void downloadable_video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		if (download_progress() == std::nullopt)
		{
			if (!available())
			{
				video_resource_context_menu_item item;
				item.name = "Download";
				item.function = [id = id()]()
					{
						ctx_.current_project->schedule_video_download(id);
					};
				items.push_back(std::move(item));
			}
			else
			{
				video_resource_context_menu_item item;
				item.name = "Remove Local File";
				item.function = [this]()
					{
						//TODO: should be done through the project so it can remove it from displayed videos or something
						remove_downloaded();
					};
				items.push_back(std::move(item));
			}
		}
	}
}
