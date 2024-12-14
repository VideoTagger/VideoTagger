#include "pch.hpp"
#include "local_video_resource.hpp"
#include "local_video_importer.hpp"
#include <video/video_stream.hpp>
#include <utils/filesystem.hpp>
#include <core/debug.hpp>

namespace vt
{
	static video_resource_metadata make_video_metadata_from_path(const std::filesystem::path& path)
	{
		video_stream video;
		if (!video.open_file(path))
		{
			throw std::runtime_error(fmt::format("Failed to open file {}", path.u8string()));
		}

		video_resource_metadata result;
		result.title = path.filename().replace_extension().u8string();
		result.width = video.width();
		result.height = video.height();
		result.fps = video.fps();
		result.duration = video.duration();

		video.close();

		auto sha256 = utils::hash::sha256_file(path);
		if (!sha256.empty())
		{
			result.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
			std::copy_n(sha256.begin(), utils::hash::sha256_byte_count, result.sha256->begin());
		}

		return result;
	}

	local_video_resource::local_video_resource(video_id_t id, std::filesystem::path path) :
		video_resource(local_video_importer::static_importer_id, id, make_video_metadata_from_path(path))
	{
		set_file_path(std::filesystem::relative(path).u8string());
	}

	local_video_resource::local_video_resource(const nlohmann::ordered_json& json) :
		video_resource(local_video_importer::static_importer_id, json)
	{
	}

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

	bool local_video_resource::update_thumbnail()
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
	}
}
