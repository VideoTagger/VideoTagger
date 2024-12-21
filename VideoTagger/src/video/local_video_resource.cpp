#include "pch.hpp"
#include "local_video_resource.hpp"
#include "local_video_importer.hpp"
#include <video/video_stream.hpp>
#include <utils/filesystem.hpp>
#include <core/debug.hpp>

namespace vt
{
	

	local_video_resource::local_video_resource(video_id_t id, std::filesystem::path path) :
		video_resource(local_video_importer::static_importer_id, id, make_video_metadata_from_path(path))
	{
		set_file_path(std::filesystem::relative(path).u8string());
	}

	local_video_resource::local_video_resource(const nlohmann::ordered_json& json) :
		video_resource(local_video_importer::static_importer_id, json) {}

	bool local_video_resource::playable() const
	{
		return std::filesystem::is_regular_file(file_path());
	}

	video_stream local_video_resource::video() const
	{
		video_stream result;
		if (!result.open_file(file_path()))
		{
			debug::panic("Failed to open video from path {}", file_path());
		}

		return result;
	}

	std::function<bool()> local_video_resource::update_thumbnail_task()
	{
		return [this]()
		{
			video_stream video;
			if (!video.open_file(file_path()))
			{
				debug::error("Failed to open video from path {}", file_path());
				return false;
			}

			gl_texture result(video.width() / 2, video.height() / 2, GL_RGB);
			video.get_thumbnail(result);
			set_thumbnail(std::move(result));

			return true;
		};
	}
}
