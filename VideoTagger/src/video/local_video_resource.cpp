#include "pch.hpp"
#include "local_video_resource.hpp"
#include "local_video_importer.hpp"
#include <video/video_stream.hpp>
#include <utils/filesystem.hpp>
#include <core/debug.hpp>
#include <core/app_context.hpp>

#include <ui/menu_items/video_resource_menu_items.hpp>

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

	void local_video_resource::context_menu_items(ui::widget_list& items)
	{
		video_resource::context_menu_items(items);

		if (playable())
		{
			items.add<ui::video_resource_menu_open_in_explorer>(id());
		}
		else
		{
			items.add<ui::video_resource_menu_locate>(id());
		}
	}
}
