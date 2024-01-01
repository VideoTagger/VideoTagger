#pragma once

#include <string>
#include <utility>
#include <set>

#include <video/video_time.hpp>

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
		timestamp_t start{};
		timestamp_t end{};
		
		tag_timestamp(timestamp_t time_start, timestamp_t time_end);
		tag_timestamp(timestamp_t time_point);

		void set(timestamp_t time_start, timestamp_t time_end);
		void set(timestamp_t time_point);

		duration_t duration() const;
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

		std::pair<iterator, bool> insert(timestamp_t time_start, timestamp_t time_end);
		std::pair<iterator, bool> insert(timestamp_t time_point);
		iterator erase(iterator it);

		std::pair<iterator, iterator> find_range(timestamp_t time_start, timestamp_t time_end) const;
		iterator find(timestamp_t time_point) const;

		iterator begin() const;
		reverse_iterator rbegin() const;
		iterator end() const;
		reverse_iterator rend() const;

		size_t size() const;

	private:
		std::set<tag_timestamp, tag_timeline_set_comparator_> timestamps_;
	};
}
