#include "pch.hpp"
#include "tag_timeline.hpp"

namespace vt
{
	tag_segment::tag_segment(timestamp time_start, timestamp time_end)
		: start{ std::min(time_start, time_end) }, end{ std::max(time_start, time_end) }
	{
	}

	tag_segment::tag_segment(timestamp time_point)
		: start{ time_point }, end{ time_point }
	{
	}

	void tag_segment::set(timestamp time_start, timestamp time_end)
	{
		start = std::min(time_start, time_end);
		end = std::max(time_start, time_end);
	}

	void tag_segment::set(timestamp time_point)
	{
		start = time_point;
		end = time_point;
	}

	std::chrono::nanoseconds tag_segment::duration() const
	{
		return end.seconds_total - start.seconds_total;
	}

	tag_segment_type tag_segment::type() const
	{
		return start == end ? tag_segment_type::timestamp : tag_segment_type::segment;
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::insert(timestamp time_start, timestamp time_end)
	{
		auto overlapping = find_range(time_start, time_end);
		
		timestamp insert_start = time_start;
		timestamp insert_end = time_end;
		
		if (overlapping.begin() != timestamps_.end())
		{
			auto last_it = std::prev(overlapping.end());
			if (overlapping.begin() == last_it and overlapping.begin()->start <= time_start and time_end <= overlapping.begin()->end)
			{
				return { overlapping.begin(), false};
			}

			insert_start = std::min(overlapping.begin()->start, time_start);
			insert_end = std::max(last_it->end, time_end);

			timestamps_.erase(overlapping.begin(), overlapping.end());
		}

		return timestamps_.emplace(insert_start, insert_end);
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::insert(timestamp time_point)
	{
		auto it = find(time_point);
		if (it != end())
		{
			return { it, false };
		}

		return timestamps_.emplace(time_point);
	}

	tag_timeline::iterator tag_timeline::erase(iterator it)
	{
		return timestamps_.erase(it);
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::replace(iterator it, timestamp new_start, timestamp new_end)
	{
		erase(it);
		return insert(new_start, new_end);
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::replace(iterator it, timestamp time_point)
	{
		erase(it);
		return insert(time_point);
	}

	iterator_range<tag_timeline::iterator> tag_timeline::find_range(timestamp time_start, timestamp time_end) const
	{
		//TODO: optimise (use lower/upper bound)
		 
		auto it = begin();

		while (it != end() and !(time_start <= it->end and time_end >= it->start))
		{
			++it;
		}
		auto result_begin = it;

		while (it != end() and (time_start <= it->end and time_end >= it->start))
		{
			++it;
		}
		auto result_end = it;

		return { result_begin, result_end };
	}

	tag_timeline::iterator tag_timeline::find(timestamp time_point) const
	{
		//TODO: optimise (use lower/upper bound)

		for (auto it = begin(); it != end(); ++it)
		{
			if (it->start <= time_point and time_point <= it->end)
			{
				return it;
			}
		}

		return end();
	}

	tag_timeline::iterator tag_timeline::begin() const
	{
		return timestamps_.begin();
	}

	tag_timeline::reverse_iterator tag_timeline::rbegin() const
	{
		return timestamps_.rbegin();
	}

	tag_timeline::iterator tag_timeline::end() const
	{
		return timestamps_.end();
	}

	tag_timeline::reverse_iterator tag_timeline::rend() const
	{
		return timestamps_.rend();
	}

	size_t tag_timeline::size() const
	{
		return timestamps_.size();
	}
	
	bool tag_timeline::empty() const
	{
		return timestamps_.empty();
	}
}
