#include "pch.hpp"
#include "video_pool.hpp"
#include <core/debug.hpp>

namespace vt
{
	video_group::video_group(const std::string& name, const std::vector<video_info>& video_ids) : display_name{ std::move(name) }, video_ids_(video_ids) {}

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

	bool video_group::empty() const
	{
		return video_ids_.empty();
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

    const video_group::container& video_group::videos() const
    {
		return video_ids_;
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

	bool video_pool::insert(std::unique_ptr<video_resource>&& vid_resource)
	{
		if (vid_resource == nullptr)
		{
			return false;
		}

		auto[_, inserted] = videos_.try_emplace(vid_resource->id(), std::move(vid_resource));
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

	video_resource& video_pool::get(video_id_t video_id)
	{
		return const_cast<video_resource&>(std::as_const(*this).get(video_id));
	}

	const video_resource& video_pool::get(video_id_t video_id) const
	{
		return *videos_.at(video_id);
	}

	std::vector<video_resource*> video_pool::get_group(const video_group& group)
	{
		std::vector<video_resource*> result;
		result.reserve(group.size());

		for (auto& vi : group)
		{
			result.push_back(&get(vi.id));
		}

		return result;
	}

	std::vector<const video_resource*> video_pool::get_group(const video_group& group) const
	{
		std::vector<const video_resource*> result;
		result.reserve(group.size());

		for (auto& vi : group)
		{
			result.push_back(&get(vi.id));
		}

		return result;
	}

	bool video_pool::contains(video_id_t video_id) const
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
}
