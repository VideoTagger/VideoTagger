#include "pch.hpp"
#include "local_video_resource.hpp"
#include "local_video_importer.hpp"
#include <video/video_stream.hpp>
#include <utils/filesystem.hpp>
#include <core/debug.hpp>
#include <core/app_context.hpp>

namespace vt
{
	local_video_resource::local_video_resource(video_id_t id, std::filesystem::path path) :
		video_resource(local_video_importer::static_importer_id, id, make_video_metadata_from_path(path))
	{
		debug::log("current: {}", std::filesystem::current_path().u8string());

		if (utils::filesystem::is_subdirectory(std::filesystem::current_path(), path))
		{
			set_file_path(std::filesystem::relative(path).u8string());
			return;
		}

		set_file_path(std::filesystem::absolute(path).u8string());
	}

	local_video_resource::local_video_resource(const nlohmann::ordered_json& json) :
		video_resource(local_video_importer::static_importer_id, json) {}

	bool local_video_resource::playable() const
	{
		return std::filesystem::is_regular_file(file_path());
	}

	void local_video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		video_resource::context_menu_items(items);

		auto absolute_path = std::filesystem::absolute(file_path());
		if (playable())
		{
			video_resource_context_menu_item item;
			item.name = fmt::format("{} Open in Explorer", icons::folder);
			item.function = [path = absolute_path]()
			{
				utils::filesystem::open_in_explorer(path.parent_path());
			};
			items.push_back(std::move(item));
		}
		else
		{
			video_resource_context_menu_item item;
			item.name = fmt::format("{} Locate File", icons::folder);
			item.function = [this, path = absolute_path.parent_path()]()
			{
				static utils::dialog_filters filters
				{
					{ "Video", utils::filesystem::concat_extensions(std::vector<std::string>(ctx_.valid_video_extensions.begin(), ctx_.valid_video_extensions.end())) },
				};

				auto result = utils::filesystem::get_file(path, filters);
				if (!result) return;

				auto hash = utils::hash::sha256_file(result.path);
				if (!hash.has_value()) return;

				if (hash != metadata().sha256)
				{
					debug::error("Selected file has different hash than the original file, can't use it as a replacement");
					return;
				}

				set_file_path(result.path.u8string());
				ctx_.is_project_dirty = true;
			};
			items.push_back(std::move(item));
		}
	}
}
