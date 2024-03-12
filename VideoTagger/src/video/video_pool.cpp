#include "video_pool.hpp"
namespace vt
{
	uint64_t video_pool::insert(const std::filesystem::path& video_path)
	{
		uint64_t video_id = utils::uuid::gen_uuid();
		video_info videoInfo = { video_path, video() };
		videos_.try_emplace(video_id, std::move(videoInfo));
		return video_id;
	}

	void video_pool::erase(uint64_t video_id)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return;
		}

		videos_.erase(it);
	}

	void video_pool::open_video(uint64_t video_id, SDL_Renderer* renderer)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return;
		}

		video_info& video_info = it->second;
		video_info.video.open_file(video_info.path, renderer);
	}

	void video_pool::close_video(uint64_t video_id)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return;
		}

		video_info& video_info = it->second;
		video_info.video.close();
	}

	video& video_pool::get(uint64_t video_id)
	{
		return const_cast<video&>(std::as_const(*this).get(video_id));
	}

	const video& video_pool::get(uint64_t video_id) const
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			throw std::runtime_error("Video ID not found.");
		}

		return it->second.video;
	}

	bool video_pool::is_open(uint64_t video_id) const
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		const video_info& video_info = it->second;
		return video_info.video.is_open();
	}

	bool video_pool::exists(uint64_t video_id) const
	{
		return videos_.count(video_id) != 0;
	}

	size_t video_pool::size() const
	{
		return videos_.size();
	}

	video_pool::iterator video_pool::begin()
	{
		return videos_.begin();
	}

	video_pool::const_iterator video_pool::begin() const
	{
		return videos_.begin();
	}

	video_pool::const_iterator video_pool::cbegin() const
	{
		return videos_.cbegin();
	}

	video_pool::iterator video_pool::end()
	{
		return videos_.end();
	}

	video_pool::const_iterator video_pool::end() const
	{
		return videos_.end();
	}

	video_pool::const_iterator video_pool::cend() const
	{
		return videos_.cend();
	}
}
