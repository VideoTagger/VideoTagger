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

	std::optional<video_resource_thumbnail> local_video_resource::generate_thumbnail()
	{
		video_stream video;
		if (!video.open_file(file_path()))
		{
			debug::error("Failed to open video from path {}", file_path());
			return std::nullopt;
		}

		constexpr int target_thumbnail_size = 256; // Thumbnail size in pixels
		float aspect_ratio = static_cast<float>(video.width()) / video.height();
		int thumbnail_width = target_thumbnail_size;
		int thumbnail_height = target_thumbnail_size;
		if (video.width() > video.height())
		{
			thumbnail_height = static_cast<int>(target_thumbnail_size / aspect_ratio);
		}
		else
		{
			thumbnail_width = static_cast<int>(target_thumbnail_size * aspect_ratio);
		}

		//TODO: calculate size differently (so that every thumbnail has approximately the same same)
		gl_texture result(thumbnail_width, thumbnail_height, GL_RGB);
		video_resource_thumbnail thumbnail;
		thumbnail.width = thumbnail_width;
		thumbnail.height = thumbnail_height;
		thumbnail.pixels.resize(thumbnail.width * thumbnail.height * 3);
		video.get_thumbnail(thumbnail.pixels, thumbnail.width, thumbnail.height);

		return std::make_optional<video_resource_thumbnail>(std::move(thumbnail));
	}
}
