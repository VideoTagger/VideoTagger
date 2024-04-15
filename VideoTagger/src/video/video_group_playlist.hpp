#pragma once
#include <deque>
#include <random>

#include "video_pool.hpp"

namespace vt
{
	struct video_group_playlist_element
	{
		bool was_played{};
		video_group_id_t group_id;
	};

	class video_group_playlist
	{
	public:
		//TODO: maybe just use vector
		using container = std::deque<video_group_playlist_element>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_group_playlist();

		iterator next();
		iterator previous();
		iterator set_current(const_iterator where);

		iterator current();
		const_iterator current() const;

		void set_shuffle(bool value);
		
		bool is_shuffled() const;

		iterator insert(const_iterator where, video_group_id_t group_id);
		void push_front(video_group_id_t group_id);
		void push_back(video_group_id_t group_id);

		void pop_front();
		void pop_back();

		iterator erase(const_iterator where);

		void clear();
		void clear_played_flags();

		video_group_playlist_element& front();
		const video_group_playlist_element& front() const;
		video_group_playlist_element& back();
		const video_group_playlist_element& back() const;
		video_group_playlist_element& at(size_t position);
		const video_group_playlist_element& at(size_t position) const;

		video_group_playlist_element& operator[](size_t position);
		const video_group_playlist_element& operator[](size_t position) const;

		size_t size() const;
		bool empty() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		container queue_;
		iterator current_element_;

		bool shuffled_;
	};
}
