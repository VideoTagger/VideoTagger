#pragma once

#include <string>
#include <utility>
#include <set>
#include <chrono>

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
		std::chrono::nanoseconds start{};
		std::chrono::nanoseconds end{};
		
		tag_timestamp(std::chrono::nanoseconds time_start, std::chrono::nanoseconds time_end);
		tag_timestamp(std::chrono::nanoseconds time_point);

		void set(std::chrono::nanoseconds time_start, std::chrono::nanoseconds time_end);
		void set(std::chrono::nanoseconds time_point);

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

		std::pair<iterator, bool> insert(std::chrono::nanoseconds time_start, std::chrono::nanoseconds time_end);
		std::pair<iterator, bool> insert(std::chrono::nanoseconds time_point);
		iterator erase(iterator it);

		std::pair<iterator, iterator> find_range(std::chrono::nanoseconds time_start, std::chrono::nanoseconds time_end) const;
		iterator find(std::chrono::nanoseconds time_point) const;

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
