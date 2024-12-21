#include "pch.hpp"
#include "video_group_playlist.hpp"

namespace vt
{
	//TODO: Use random from uuid/make a "random" class
	static std::mt19937_64 random_engine(std::random_device{}());

	video_group_playlist::video_group_playlist() : shuffled_{ false }, current_element_{ end() } {}

	video_group_playlist::iterator video_group_playlist::peek_next()
	{
		iterator next_element = current_element_;

		if (next_element == end())
		{
			return end();
		}

		if (!shuffled_)
		{
			next_element++;
		}
		else
		{
			if (!shuffled_history_next.empty())
			{
				next_element = begin() + shuffled_history_next.top();
			}
			else
			{
				auto shuffled_it = std::find_if(
					shuffled_indices_.begin(), shuffled_indices_.end(),
					[current_element_index = next_element - begin()](size_t index)
					{
						return index == current_element_index;
					}
				);

				if (shuffled_it == shuffled_indices_.end())
				{
					return end();
				}

				shuffled_it++;
				if (shuffled_it == shuffled_indices_.end())
				{
					next_element = end();
				}
				else
				{
					next_element = begin() + *shuffled_it;
				}
			}
		}

		return next_element;
	}

	video_group_playlist::const_iterator video_group_playlist::peek_next() const
	{
		return const_cast<video_group_playlist&>(*this).peek_next();
	}

	video_group_playlist::iterator video_group_playlist::peek_previous()
	{
		iterator previous_element = current_element_;

		if (!shuffled_ and previous_element != begin())
		{
			previous_element--;
		}
		else
		{
			if (!shuffled_history_previous.empty())
			{
				previous_element = begin() + shuffled_history_previous.top();
			}
		}

		return previous_element;
	}

	video_group_playlist::const_iterator video_group_playlist::peek_previous() const
	{
		return const_cast<video_group_playlist&>(*this).peek_previous();
	}

	video_group_playlist::iterator video_group_playlist::next()
	{
		size_t current_index = current_element_ - begin();

		current_element_ = peek_next();

		if (!shuffled_)
		{
			reshuffle();
		}
		else
		{
			shuffled_history_previous.push(current_index);

			if (!shuffled_history_next.empty())
			{
				shuffled_history_next.pop();
			}
		}

		return current_element_;
	}

	video_group_playlist::iterator video_group_playlist::previous()
	{
		size_t current_index = current_element_ - begin();

		current_element_ = peek_previous();
		if (!shuffled_)
		{
			reshuffle();
		}
		else if (!shuffled_history_previous.empty())
		{
			shuffled_history_next.push(current_index);
			shuffled_history_previous.pop();
		}
		
		return current_element_;
	}

	video_group_playlist::iterator video_group_playlist::set_current(const_iterator where)
	{
		current_element_ = begin() + (where - begin());

		reshuffle();

		return current_element_;
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

		auto result = groups_.insert(where, group_id);
		current_element_ = begin() + current_index;

		reshuffle();

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
		auto result = groups_.erase(where);

		current_element_ = begin() + current_index;

		reshuffle();

		return result;
	}

	void video_group_playlist::clear()
	{
		groups_.clear();
		shuffled_indices_.clear();
		current_element_ = end();
		shuffled_history_next = {};
		shuffled_history_previous = {};
	}

	void video_group_playlist::reshuffle()
	{
		if (groups_.empty()) return;

		shuffled_history_next = {};
		shuffled_history_previous = {};

		shuffled_indices_.clear();
		shuffled_indices_.reserve(groups_.size());

		if (current_element_ != end())
		{
			size_t current_index = current_element_ - begin();

			shuffled_indices_.push_back(current_index);
			for (size_t i = 0; i < groups_.size(); i++)
			{
				if (i == current_index)
				{
					continue;
				}

				shuffled_indices_.push_back(i);
			}

			std::shuffle(shuffled_indices_.begin() + 1, shuffled_indices_.end(), random_engine);
		}
		else
		{
			for (size_t i = 0; i < groups_.size(); i++)
			{
				shuffled_indices_.push_back(i);
			}

			std::shuffle(shuffled_indices_.begin(), shuffled_indices_.end(), random_engine);
		}
	}

	video_group_id_t& video_group_playlist::front()
	{
		return groups_.front();
	}

	const video_group_id_t& video_group_playlist::front() const
	{
		return groups_.front();
	}

	video_group_id_t& video_group_playlist::back()
	{
		return groups_.back();
	}

	const video_group_id_t& video_group_playlist::back() const
	{
		return groups_.back();
	}

	video_group_id_t& video_group_playlist::at(size_t position)
	{
		return groups_.at(position);
	}

	const video_group_id_t& video_group_playlist::at(size_t position) const
	{
		return groups_.at(position);
	}

	video_group_id_t& video_group_playlist::operator[](size_t position)
	{
		return groups_.operator[](position);
	}

	const video_group_id_t& video_group_playlist::operator[](size_t position) const
	{
		return groups_.operator[](position);
	}

	size_t video_group_playlist::size() const
	{
		return groups_.size();
	}

	bool video_group_playlist::empty() const
	{
		return groups_.empty();
	}

    const std::vector<video_group_id_t>& video_group_playlist::groups() const
    {
		return groups_;
    }

	video_group_playlist::iterator video_group_playlist::begin()
	{
		return groups_.begin();
	}

	video_group_playlist::const_iterator video_group_playlist::begin() const
	{
		return groups_.cbegin();
	}

	video_group_playlist::const_iterator video_group_playlist::cbegin() const
	{
		return groups_.cbegin();
	}

	video_group_playlist::iterator video_group_playlist::end()
	{
		return groups_.end();
	}

	video_group_playlist::const_iterator video_group_playlist::end() const
	{
		return groups_.cend();
	}

	video_group_playlist::const_iterator video_group_playlist::cend() const
	{
		return groups_.cend();
	}
}
