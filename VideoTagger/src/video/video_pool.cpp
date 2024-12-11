#include "pch.hpp"
#include "video_pool.hpp"
#include <core/debug.hpp>

namespace vt
{
	video_group::video_group(std::string name, std::vector<video_info> video_ids)
		: display_name{ std::move(name) }, video_ids_(video_ids)
	{}

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
		auto it = find(video_id);
		if (it == end())
		{
			return false;
		}

		video_ids_.erase(it);
		return true;
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

	video_group::video_info& video_group::at(size_t index)
	{
		return video_ids_.at(index);
	}

	const video_group::video_info& video_group::at(size_t index) const
	{
		return video_ids_.at(index);
	}

	video_group::video_info& video_group::operator[](size_t index)
	{
		return video_ids_[index];
	}

	const video_group::video_info& video_group::operator[](size_t index) const
	{
		return video_ids_[index];
	}

	segment_storage& video_group::segments()
	{
		return segments_;
	}

	const segment_storage& video_group::segments() const
	{
		return segments_;
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

		auto[_, inserted] = videos_.try_emplace(video_id, video_metadata{ video_path });
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

	bool video_pool::open_video(video_id_t video_id)
	{
		auto it = videos_.find(video_id);
		if (it == videos_.end())
		{
			return false;
		}

		video_metadata& video_info = it->second;
		return video_info.open_video();
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

	std::vector<video_id_t> video_pool::open_group(const video_group& group)
	{
		std::vector<video_id_t> result;

		for (auto& vi : group)
		{
			if (!open_video(vi.id))
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

	bool video_pool::empty() const
	{
		return videos_.empty();
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
	}

	bool video_pool::video_metadata::update_data()
	{
		bool was_open = video.is_open();
		if (!was_open)
		{
			if (!video.open_file(path))
			{
				return false;
			}
		}

		width = video.width();
		height = video.height();
		duration = video.duration();
		fps = video.fps();
		//update_thumbnail(gl_ctx);

		if (!was_open)
		{
			video.close();
		}

		return true;
	}

	bool video_pool::video_metadata::update_thumbnail()
	{
		bool was_open = video.is_open();
		if (!was_open)
		{
			if (!video.open_file(path))
			{
				return false;
			}
		}

		

		thumbnail = gl_texture(video.width() / 2, video.height() / 2, GL_RGB);
		video.get_thumbnail(*thumbnail);

		if (!was_open)
		{
			video.close();
		}

		return true;
	}

	bool video_pool::video_metadata::open_video()
	{
		return video.open_file(path);
	}

	void video_pool::video_metadata::close_video()
	{
		video.close();
	}
}
