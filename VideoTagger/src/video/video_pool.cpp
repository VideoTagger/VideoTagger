#include "video_pool.hpp"
namespace vt
{
	uint64_t video_pool::insert(const std::filesystem::path& video_path)
	{
		uint64_t video_id = utils::uuid::gen_uuid();
		video_info videoInfo = { video_path, video() };
		video_map_.try_emplace(video_id, videoInfo);
		return video_id;
	}

	void video_pool::open_video(uint64_t video_id)
	{
		if (exists(video_id))
		{
			video_info& videoInfo = video_map_[video_id];
			videoInfo.videoObject.open_file(videoInfo.videoPath, renderer_);
		}
		else
		{
			throw std::runtime_error("ID not found");
		}
	}

	void video_pool::close_video(uint64_t video_id)
	{
		if (exists(video_id))
		{
			video_info& videoInfo = video_map_[video_id];
			videoInfo.videoObject.close();
		}
		else
		{
			throw std::runtime_error("ID not found");
		}
	}

	bool video_pool::is_open(uint64_t video_id) const
	{
		if (exists(video_id))
		{
			const video_info& videoInfo = video_map_.at(video_id);
			return videoInfo.videoObject.is_open();
		}
		else
		{
			throw std::runtime_error("ID not found");
		}
	}

	bool video_pool::exists(uint64_t video_id) const
	{
		if (video_map_.count(video_id) > 0)
		{
			return true;
		}
		return false;
	}

	size_t video_pool::size() const
	{
		return video_map_.size();
	}
}
