#pragma once

#include <string>
#include <utility>
#include <set>
#include <chrono>

#include <utils/timestamp.hpp>

namespace vt
{
	//TODO: maybe think of something better than "timestamp" for this
	enum class tag_timestamp_type
	{
		segment,
		point
	};

	struct tag_timestamp
	{
		timestamp start{};
		timestamp end{};
		
		tag_timestamp(timestamp time_start, timestamp time_end);
		tag_timestamp(timestamp time_point);

		void set(timestamp time_start, timestamp time_end);
		void set(timestamp time_point);

		std::chrono::nanoseconds duration() const;
		tag_timestamp_type type() const;
	};

	class tag_timeline
	{
		struct tag_timeline_set_comparator_
		{
			bool operator()(const tag_timestamp& lhs, const tag_timestamp& rhs) const
			{
				return lhs.start < rhs.start;
			}
		};

	public:
		using iterator = std::set<tag_timestamp, tag_timeline_set_comparator_>::iterator;
		using reverse_iterator = std::set<tag_timestamp, tag_timeline_set_comparator_>::reverse_iterator;

		std::pair<iterator, bool> insert(timestamp time_start, timestamp time_end);
		std::pair<iterator, bool> insert(timestamp time_point);
		iterator erase(iterator it);

		//TODO: add replace timestamp function (erase + insert)

		std::pair<iterator, iterator> find_range(timestamp time_start, timestamp time_end) const;
		iterator find(timestamp time_point) const;

		iterator begin() const;
		reverse_iterator rbegin() const;
		iterator end() const;
		reverse_iterator rend() const;

		size_t size() const;
		bool empty() const;

	private:
		std::set<tag_timestamp, tag_timeline_set_comparator_> timestamps_;
	};
}
