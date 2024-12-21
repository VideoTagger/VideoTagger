#include "pch.hpp"
#include "tag_timeline.hpp"

namespace vt
{
	tag_segment::tag_segment(timestamp time_start, timestamp time_end, const attribute_instance_container& attributes) : start{ std::min(time_start, time_end) }, end{ std::max(time_start, time_end) }, attributes{ attributes } {}
	tag_segment::tag_segment(timestamp time_point, const attribute_instance_container& attributes) : start{ time_point }, end{ time_point }, attributes{ attributes } {}

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
		return (end - start).total_milliseconds;
	}

	tag_segment_type tag_segment::type() const
	{
		return start == end ? tag_segment_type::timestamp : tag_segment_type::segment;
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::insert(timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes)
	{
		auto prepare_result = prepare_insert(time_start, time_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			if (!can_insert)
			{
				return { prepare_result->first.begin(), false };
			}

			segments_.erase(overlapping.begin(), overlapping.end());
		}

		return segments_.emplace(time_start, time_end, attributes);
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::insert(timestamp time_point, const tag_segment::attribute_instance_container& attributes)
	{
		auto prepare_result = prepare_insert(time_point);
		if (prepare_result.has_value())
		{
			return { *prepare_result, false };
		}

		return segments_.emplace(time_point, attributes);
	}

	tag_timeline::iterator tag_timeline::erase(iterator it)
	{
		return segments_.erase(it);
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::replace(iterator it, timestamp new_start, timestamp new_end)
	{
		auto node = segments_.extract(it);

		auto prepare_result = prepare_insert(new_start, new_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			if (!can_insert)
			{
				segments_.insert(std::move(node));
				return { prepare_result->first.begin(), false };
			}

			segments_.erase(overlapping.begin(), overlapping.end());
		}

		auto& value = node.value();
		value.start = new_start;
		value.end = new_end;

		auto insert_result = segments_.insert(std::move(node));

		return { insert_result.position, insert_result.inserted };
	}

	std::pair<tag_timeline::iterator, bool> tag_timeline::replace(iterator it, timestamp time_point)
	{
		auto prepare_result = prepare_insert(time_point);
		if (prepare_result.has_value())
		{
			return { *prepare_result, false };
		}

		auto node = segments_.extract(it);
		auto& value = node.value();
		value.start = time_point;
		value.end = time_point;

		auto insert_result = segments_.insert(std::move(node));

		return { insert_result.position, insert_result.inserted };
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
		return segments_.begin();
	}

	tag_timeline::reverse_iterator tag_timeline::rbegin() const
	{
		return segments_.rbegin();
	}

	tag_timeline::iterator tag_timeline::end() const
	{
		return segments_.end();
	}

	tag_timeline::reverse_iterator tag_timeline::rend() const
	{
		return segments_.rend();
	}

	size_t tag_timeline::size() const
	{
		return segments_.size();
	}
	
	bool tag_timeline::empty() const
	{
		return segments_.empty();
	}

	std::optional<std::pair<iterator_range<tag_timeline::iterator>, bool>> tag_timeline::prepare_insert(timestamp& time_start, timestamp& time_end)
	{
		auto overlapping = find_range(time_start, time_end);
		if (!overlapping.empty())
		{
			auto last_it = std::prev(overlapping.end());
			if (overlapping.begin() == last_it and overlapping.begin()->start <= time_start and time_end <= overlapping.begin()->end)
			{
				return std::make_pair(overlapping, false);
			}

			time_start = std::min(overlapping.begin()->start, time_start);
			time_end = std::max(last_it->end, time_end);

			return std::make_pair(overlapping, true);
		}

		return std::nullopt;
	}

	std::optional<tag_timeline::iterator> tag_timeline::prepare_insert(timestamp ts)
	{
		auto it = find(ts);
		if (it != end())
		{
			return it;
		}

		return std::nullopt;
	}
}
