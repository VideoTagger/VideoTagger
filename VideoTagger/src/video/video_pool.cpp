#include "video_pool.hpp"
#include <utils/hash.hpp>


namespace vt
{
	video_group::video_group(std::vector<video_info> video_ids) : video_ids_(video_ids) {}

	bool video_group::insert(video_info video_info)
	{
		if (contains(video_info.id))
		{
			return false;
		}

		video_ids_.push_back(video_info);
		return true;
	}

	bool video_group::erase(video_id_t video_id)
	{
		return false;
	}

	bool video_group::contains(video_id_t video_id) const
	{
		return find(video_id) != end();
	}

	size_t video_group::size() const
	{
		return video_ids_.size();
	}

	video_group::iterator video_group::find(video_id_t video_id)
	{
		for (auto it = begin(); it != end(); ++it)
		{
			if (it->id == video_id)
			{
				return it;
			}
		}

		return end();
	}

	video_group::const_iterator video_group::find(video_id_t video_id) const
	{
		for (auto it = begin(); it != end(); ++it)
		{
			if (it->id == video_id)
			{
				return it;
			}
		}

		return end();
	}

	video_group::iterator video_group::begin()
	{
		return video_ids_.begin();
	}

	video_group::const_iterator video_group::begin() const
	{
		return video_ids_.begin();
	}

	video_group::const_iterator video_group::cbegin() const
	{
		return video_ids_.cbegin();
	}

	video_group::iterator video_group::end()
	{
		return video_ids_.end();
	}

	video_group::const_iterator video_group::end() const
	{
		return video_ids_.end();
	}

	video_group::const_iterator video_group::cend() const
	{
		return video_ids_.cend();
	}

	video_id_t video_pool::insert(const std::filesystem::path& video_path)
	{
		video_id_t video_id = utils::hash::fnv_hash(video_path); //utils::uuid::get()
		
		if (!insert(video_id, video_path))
		{
			return 0;
		}

		return video_id;
	}

	bool video_pool::insert(video_id_t video_id, const std::filesystem::path& video_path)
	{
		{
			//TODO: maybe add a function to check if a file is a video file
			// some files don't fail to open despite not being a video
			video_decoder decoder;
			if (!decoder.open(video_path))
			{
				return false;
			}
		}

		video_metadata video_info = { video_path, video_stream() };
		auto[_, inserted] = videos_.try_emplace(video_id, std::move(video_info));
		return inserted;
	}

	bool video_pool::erase(video_id_t video_id)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		videos_.erase(it);
		return true;
	}

	bool video_pool::open_video(video_id_t video_id, SDL_Renderer* renderer)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		video_metadata& video_info = it->second;
		return video_info.open_video(renderer);
	}

	bool video_pool::close_video(video_id_t video_id)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		video_metadata& video_info = it->second;
		video_info.close_video();
		return true;
	}

	std::vector<video_id_t> video_pool::open_group(const video_group& group, SDL_Renderer* renderer)
	{
		std::vector<video_id_t> result;

		for (auto& vi : group)
		{
			if (!open_video(vi.id, renderer))
			{
				result.push_back(vi.id);
			}
		}

		return result;
	}

	std::vector<video_id_t> video_pool::close_group(const video_group& group)
	{
		std::vector<video_id_t> result;

		for (auto& vi : group)
		{
			if (!close_video(vi.id))
			{
				result.push_back(vi.id);
			}
		}

		return result;
	}

	video_pool::video_metadata* video_pool::get(video_id_t video_id)
	{
		return const_cast<video_metadata*>(std::as_const(*this).get(video_id));
	}

	const video_pool::video_metadata* video_pool::get(video_id_t video_id) const
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	std::vector<video_pool::video_metadata*> video_pool::get_group(const video_group& group)
	{
		std::vector<video_metadata*> result;
		result.reserve(group.size());

		for (auto& vi : group)
		{
			result.push_back(get(vi.id));
		}

		return result;
	}

	std::vector<const video_pool::video_metadata*> video_pool::get_group(const video_group& group) const
	{
		std::vector<const video_metadata*> result;
		result.reserve(group.size());

		for (auto& vi : group)
		{
			result.push_back(get(vi.id));
		}

		return result;
	}

	bool video_pool::is_open(video_id_t video_id) const
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		const video_metadata& video_info = it->second;
		return video_info.video.is_open();
	}

	bool video_pool::exists(video_id_t video_id) const
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


	video_pool::video_metadata::~video_metadata()
	{
		if (thumbnail != nullptr)
		{
			SDL_DestroyTexture(thumbnail);
		}
	}

	bool video_pool::video_metadata::update_data(SDL_Renderer* renderer)
	{
		bool was_open = video.is_open();
		if (!was_open)
		{
			if (!video.open_file(path, renderer))
			{
				return false;
			}
		}

		width = video.width();
		height = video.height();
		duration = video.duration();
		fps = video.fps();
		update_thumbnail(renderer);

		if (!was_open)
		{
			video.close();
		}

		return true;
	}

	bool video_pool::video_metadata::update_thumbnail(SDL_Renderer* renderer)
	{
		bool was_open = video.is_open();
		if (!was_open)
		{
			if (!video.open_file(path, renderer))
			{
				return false;
			}
		}

		if (thumbnail != nullptr)
		{
			SDL_DestroyTexture(thumbnail);
		}

		thumbnail = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, video.width() / 2, video.height() / 2);
		if (thumbnail == nullptr)
		{
			return false;
		}

		video.get_thumbnail(renderer, thumbnail);

		if (!was_open)
		{
			video.close();
		}

		return true;
	}

	bool video_pool::video_metadata::open_video(SDL_Renderer* renderer)
	{
		return video.open_file(path, renderer);
	}

	void video_pool::video_metadata::close_video()
	{
		video.close();
	}
}
