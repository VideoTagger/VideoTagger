#include "pch.hpp"
#include "video_group_playlist.hpp"

namespace vt
{
	video_group_playlist::video_group_playlist()
		: shuffled_{ false }, current_element_{ end() }, random_engine_{ std::random_device{}() }
	{
	}

	video_group_playlist::iterator video_group_playlist::next()
	{
		if (current_element_ == end())
		{
			return current_element_;
		}

		if (!shuffled_)
		{
			return ++current_element_;
		}

		std::vector<size_t> item_pool;
		item_pool.reserve(size());

		for (size_t i = 0; i < size(); i++)
		{
			if (queue_[i].was_played)
			{
				continue;
			}

			item_pool.push_back(i);
		}

		if (item_pool.empty())
		{
			current_element_ = end();
			return current_element_;
		}

		std::uniform_int_distribution<size_t> distribution(0, item_pool.size() - 1);
		current_element_ = begin() + distribution(random_engine_);

		return current_element_;
	}

	video_group_playlist::iterator video_group_playlist::previous()
	{
		if (current_element_ == begin())
		{
			return current_element_;
		}

		return --current_element_;
	}

	video_group_playlist::iterator video_group_playlist::set_current(const_iterator where)
	{
		return current_element_ = begin() + (where - begin());
	}

	video_group_playlist::iterator video_group_playlist::current()
	{
		return current_element_;
	}

	video_group_playlist::const_iterator video_group_playlist::current() const
	{
		return current_element_;
	}

	void video_group_playlist::set_shuffle(bool value)
	{
		shuffled_ = value;
	}

	bool video_group_playlist::is_shuffled() const
	{
		return shuffled_;
	}

	video_group_playlist::iterator video_group_playlist::insert(const_iterator where, video_group_id_t group_id)
	{
		size_t current_index = current_element_ - begin();
		if (current_element_ >= where)
		{
			current_index += 1;
		}

		video_group_playlist_element item;
		item.group_id = group_id;
		item.was_played = false;
		auto result = queue_.insert(where, item);

		current_element_ = begin() + current_index;

		return result;
	}

	void video_group_playlist::push_front(video_group_id_t group_id)
	{
		insert(begin(), group_id);
	}

	void video_group_playlist::push_back(video_group_id_t group_id)
	{
		insert(end(), group_id);
	}

	void video_group_playlist::pop_front()
	{
		erase(begin());
	}

	void video_group_playlist::pop_back()
	{
		erase(end() - 1);
	}

	video_group_playlist::iterator video_group_playlist::erase(const_iterator where)
	{
		size_t current_index = current_element_ - begin();
		if (current_element_ > where)
		{
			current_index -= 1;
		}

		bool erased_current = where == current_element_;
		auto result = queue_.erase(where);

		current_element_ = begin() + current_index;

		if (erased_current and current_element_ != end())
		{
			current_element_->was_played = true;
		}

		return result;
	}

	void video_group_playlist::clear()
	{
		queue_.clear();
		current_element_ = end();
	}

	void video_group_playlist::clear_played_flags()
	{
		for (auto& item : queue_)
		{
			item.was_played = false;
		}
	}

	video_group_playlist_element& video_group_playlist::front()
	{
		return queue_.front();
	}

	const video_group_playlist_element& video_group_playlist::front() const
	{
		return queue_.front();
	}

	video_group_playlist_element& video_group_playlist::back()
	{
		return queue_.back();
	}

	const video_group_playlist_element& video_group_playlist::back() const
	{
		return queue_.back();
	}

	video_group_playlist_element& video_group_playlist::at(size_t position)
	{
		return queue_.at(position);
	}

	const video_group_playlist_element& video_group_playlist::at(size_t position) const
	{
		return queue_.at(position);
	}

	video_group_playlist_element& video_group_playlist::operator[](size_t position)
	{
		return queue_.operator[](position);
	}

	const video_group_playlist_element& video_group_playlist::operator[](size_t position) const
	{
		return queue_.operator[](position);
	}

	size_t video_group_playlist::size() const
	{
		return queue_.size();
	}

	bool video_group_playlist::empty() const
	{
		return queue_.empty();
	}

	video_group_playlist::iterator video_group_playlist::begin()
	{
		return queue_.begin();
	}

	video_group_playlist::const_iterator video_group_playlist::begin() const
	{
		return queue_.cbegin();
	}

	video_group_playlist::const_iterator video_group_playlist::cbegin() const
	{
		return queue_.cbegin();
	}

	video_group_playlist::iterator video_group_playlist::end()
	{
		return queue_.end();
	}

	video_group_playlist::const_iterator video_group_playlist::end() const
	{
		return queue_.cend();
	}

	video_group_playlist::const_iterator video_group_playlist::cend() const
	{
		return queue_.cend();
	}
}
