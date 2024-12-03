#pragma once
#include <stack>

#include <utils/json.hpp>
#include "video_pool.hpp"

namespace vt
{
	class video_group_playlist
	{
	public:
		using value_type = video_group_id_t;
		using iterator = std::vector<video_group_id_t>::iterator;
		using const_iterator = std::vector<video_group_id_t>::const_iterator;

		video_group_playlist();

		//TODO: peek next/previous

		iterator peek_next();
		const_iterator peek_next() const;
		iterator peek_previous();
		const_iterator peek_previous() const;

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
		void reshuffle();

		video_group_id_t& front();
		const video_group_id_t& front() const;
		video_group_id_t& back();
		const video_group_id_t& back() const;
		video_group_id_t& at(size_t position);
		const video_group_id_t& at(size_t position) const;

		video_group_id_t& operator[](size_t position);
		const video_group_id_t& operator[](size_t position) const;

		size_t size() const;
		bool empty() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		std::vector<video_group_id_t> videos_;
		std::vector<size_t> shuffled_indices_;
		std::stack<size_t> shuffled_history_next;
		std::stack<size_t> shuffled_history_previous;

		iterator current_element_;

		bool shuffled_;
	};

	inline void to_json(nlohmann::ordered_json& json, const video_group_playlist& vp)
	{
		json = nlohmann::json::array();
		for (const auto& e : vp)
		{
			json.push_back(e);
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, video_group_playlist& vp)
	{
		for (auto& e : json)
		{
			vp.push_back(e.get<video_group_id_t>());
		}
	}
}
