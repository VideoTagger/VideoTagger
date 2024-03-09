#include "video_pool.hpp"
namespace vt
{
	uint64_t VideoPool::insert(const std::filesystem::path& video_path)
	{
		uint64_t video_id = utils::uuid::gen_uuid();
		VideoInfo videoInfo = { video_path, video() };
		video_map_[video_id] = videoInfo;
		return video_id;
	}

	video& VideoPool::get_video(uint64_t video_id)
	{
		if (exists(video_id))
		{
			VideoInfo& videoInfo = video_map_[video_id];
			videoInfo.videoObject.open_file(videoInfo.videoPath, renderer_);
			return videoInfo.videoObject;
		}
		else
		{
			throw std::runtime_error("ID not found");
		}
	}

	bool VideoPool::exists(uint64_t video_id) const
	{
		if (video_map_.count(video_id) > 0)
		{
			return true;
		}
		throw std::runtime_error("ID not found");
	}

	size_t VideoPool::size() const
	{
		return video_map_.size();
	}
}
