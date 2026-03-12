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
		if (path.is_relative())
		{
			set_file_path(path.u8string());
			return;
		}

		auto current_root = std::filesystem::current_path().root_name();
		if (path.root_name() == current_root)
		{
			set_file_path(std::filesystem::relative(path).u8string());
			return;
		}

		set_file_path(path.u8string());
	}

	local_video_resource::local_video_resource(const nlohmann::ordered_json& json) :
		video_resource(local_video_importer::static_importer_id, json) {}

	bool local_video_resource::playable() const
	{
		return std::filesystem::is_regular_file(file_path());
	}
}
