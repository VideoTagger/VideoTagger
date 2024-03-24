#pragma once

#include <string>
#include <utility>
#include <set>
#include <chrono>
#include <unordered_map>

#include <utils/timestamp.hpp>
#include <utils/iterator_range.hpp>

namespace vt
{
	//TODO: maybe think of something better than "timestamp" for this
	enum class tag_segment_type
	{
		point,
		segment
	};

	struct tag_segment
	{
		timestamp start{};
		timestamp end{};
		
		tag_segment(timestamp time_start, timestamp time_end);
		tag_segment(timestamp time_point);

		void set(timestamp time_start, timestamp time_end);
		void set(timestamp time_point);

		std::chrono::nanoseconds duration() const;
		tag_segment_type type() const;
	};

	class tag_timeline
	{
		struct tag_timeline_set_comparator_
		{
			bool operator()(const tag_segment& lhs, const tag_segment& rhs) const
			{
				return lhs.start < rhs.start;
			}
		};

	public:
		using iterator = std::set<tag_segment, tag_timeline_set_comparator_>::iterator;
		using reverse_iterator = std::set<tag_segment, tag_timeline_set_comparator_>::reverse_iterator;

		std::pair<iterator, bool> insert(timestamp time_start, timestamp time_end);
		std::pair<iterator, bool> insert(timestamp time_point);
		iterator erase(iterator it);

		//will invalidate it
		std::pair<iterator, bool> replace(iterator it, timestamp new_start, timestamp new_end);
		//will invalidate it
		std::pair<iterator, bool> replace(iterator it, timestamp time_point);

		iterator_range<iterator> find_range(timestamp time_start, timestamp time_end) const;
		iterator find(timestamp time_point) const;

		iterator begin() const;
		reverse_iterator rbegin() const;
		iterator end() const;
		reverse_iterator rend() const;

		size_t size() const;
		bool empty() const;

	private:
		std::set<tag_segment, tag_timeline_set_comparator_> timestamps_;
	};

	//key: tag name
	using segment_storage = std::unordered_map<std::string, tag_timeline>;
}
