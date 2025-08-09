#include "pch.hpp"
#include "tag_timeline.hpp"
#include <utils/random.hpp>

namespace vt
{
	tag_segment::tag_segment(timestamp time_start, timestamp time_end, const attribute_instance_container& attributes) : start{ std::min(time_start, time_end) }, end{ std::max(time_start, time_end) }, attributes{ attributes } {}
	tag_segment::tag_segment(timestamp ts, const attribute_instance_container& attributes) : start{ ts }, end{ ts }, attributes{ attributes } {}

    void tag_segment::set(timestamp time_start, timestamp time_end)
	{
		start = std::min(time_start, time_end);
		end = std::max(time_start, time_end);
	}

	void tag_segment::set(timestamp ts)
	{
		start = ts;
		end = ts;
	}

	std::chrono::milliseconds tag_segment::duration() const
	{
		return (end - start).total_milliseconds;
	}

	tag_segment_type tag_segment::type() const
	{
		return start == end ? tag_segment_type::timestamp : tag_segment_type::segment;
	}

    bool tag_segment::is_timestamp() const
    {
        return type() == tag_segment_type::timestamp;
    }

	std::pair<segment_id, bool> tag_timeline::insert(timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes)
	{
		auto prepare_result = prepare_insert(time_start, time_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			if (!can_insert)
			{
				return { prepare_result->first.begin()->id, false };
			}

			erase(overlapping.begin(), overlapping.end());
		}

		auto id = utils::random::get<segment_id>(); //TODO: Maybe check for duplicates
		auto insert_it = segments_.emplace(lower_bound(time_start), id, time_start, time_end, attributes);
		update_id_map(insert_it + 1, segments_.end(), 1);
		id_map_.try_emplace(id, insert_it - segments_.begin());
		return { id, true };
	}

	std::pair<segment_id, bool> tag_timeline::insert(timestamp ts, const tag_segment::attribute_instance_container& attributes)
	{
		auto prepare_result = prepare_insert(ts);
		if (prepare_result.has_value())
		{
			return { (*prepare_result)->id, false };
		}

		auto id = utils::random::get<segment_id>(); //TODO: Maybe check for duplicates
		auto insert_it = segments_.emplace(lower_bound(ts), id, ts, attributes);
		update_id_map(insert_it + 1, segments_.end(), 1);
		id_map_.try_emplace(id, insert_it - segments_.begin());
		return { id, true };
	}

	void tag_timeline::erase(segment_id id)
	{
		auto index = id_map_.at(id);
		auto it = segments_.begin() + index;
		it = segments_.erase(it);
		id_map_.erase(id);
		update_id_map(it, segments_.end(), -1);
	}

	tag_timeline::iterator tag_timeline::erase(iterator it)
	{
		auto id = it->id;
		it = segments_.erase(it);
		id_map_.erase(id);
		update_id_map(it, segments_.end(), -1);
		return it;
	}

	tag_timeline::iterator tag_timeline::erase(iterator it_begin, iterator it_end)
	{
		for (auto it = it_begin; it != it_end; ++it)
		{
			id_map_.erase(it->id);
		}
		auto update_it = segments_.erase(it_begin, it_end);
		update_id_map(update_it, segments_.end(), -static_cast<int64_t>(std::distance(it_begin, it_end)));
		return update_it;
	}

	std::pair<segment_id, bool> tag_timeline::move(segment_id id, timestamp new_start, timestamp new_end)
	{
		if (new_start == new_end)
		{
			return move(id, new_start);
		}

		timestamp prepare_start = new_start;
		timestamp prepare_end = new_end;
		auto prepare_result = prepare_insert(prepare_start, prepare_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			// If target location is fully contained in the overlapping segment and its not the moved segment, just remove the moved segment.
			if (!can_insert and overlapping.begin()->id != id)
			{
				auto result_id = prepare_result->first.begin()->id;
				erase(id);
				return { result_id, false };
			}

			// If the target location isn't overlapping only with the moved segment
			if (!(overlapping.size() == 1 and overlapping.begin()->id == id))
			{
				// If the first overlapping segment is the moved segment, we can't use its start as the new start.
				if (overlapping.begin()->id == id)
				{
					new_end = prepare_end;
				}
				// If the last overlapping segment is the moved segment, we can't use its end as the new end.
				else if (std::prev(overlapping.end())->id == id)
				{
					new_start = prepare_start;
				}
				else
				{
					new_start = prepare_start;
					new_end = prepare_end;
				}

				erase_if(overlapping.begin(), overlapping.end(), [id](const auto& obj)
				{
					return obj.id != id;
				});
			}
		}

		auto moved_segment = std::move(segments_.at(id_map_.at(id)).segment);
		erase(id);
		moved_segment.set(new_start, new_end);
		insert_no_check(id, std::move(moved_segment));

		return { id, true };
	}

	std::pair<segment_id, bool> tag_timeline::move(segment_id id, timestamp ts)
	{
		auto prepare_result = prepare_insert(ts);
		if (prepare_result.has_value())
		{
			auto result_id = (*prepare_result)->id;
			erase(id);
			
			return { result_id, false };
		}

		auto moved_segment = std::move(segments_.at(id_map_.at(id)).segment);
		erase(id);
		moved_segment.set(ts);
		insert_no_check(id, std::move(moved_segment));
		
		return { id, true };
	}

	iterator_range<tag_timeline::iterator> tag_timeline::find_range(timestamp time_start, timestamp time_end) const
	{
		if (empty())
		{
			return { end(), end() };
		}

		auto range_begin_it = lower_bound(time_start);
		if (range_begin_it != begin())
		{
			auto prev_it = std::prev(range_begin_it);
			if (prev_it->segment.end >= time_start)
			{
				range_begin_it = prev_it;
			}
		}

		auto range_end_it = upper_bound(time_end);

		return { range_begin_it, range_end_it };
	}

	tag_timeline::iterator tag_timeline::find(timestamp ts) const
	{
		if (empty())
		{
			return end();
		}

		auto it = lower_bound(ts);
		if (it == begin())
		{
			if (it->segment.start == ts)
			{
				return it;
			}
			return end();
		}

		it = std::prev(it);
		if (it->segment.end < ts)
		{
			return end();
		}

		return it;
	}

	const tag_segment& tag_timeline::at(segment_id id) const
	{
		return segments_.at(id_map_.at(id)).segment;
	}

	tag_timeline::iterator tag_timeline::begin() const
	{
		return segments_.begin();
	}

	tag_timeline::iterator tag_timeline::end() const
	{
		return segments_.end();
	}

	tag_timeline::reverse_iterator tag_timeline::rbegin() const
	{
		return segments_.rbegin();
	}

	tag_timeline::reverse_iterator tag_timeline::rend() const
	{
		return segments_.rend();
	}

	bool tag_timeline::is_id_valid(segment_id id) const
	{
		return id_map_.count(id) != 0;
	}

	size_t tag_timeline::size() const
	{
		return segments_.size();
	}
	
	bool tag_timeline::empty() const
	{
		return segments_.empty();
	}

	std::optional<std::pair<iterator_range<tag_timeline::iterator>, bool>> tag_timeline::prepare_insert(timestamp& time_start, timestamp& time_end) const
	{
		auto overlapping = find_range(time_start, time_end);
		if (!overlapping.empty())
		{
			const auto& first_segment = overlapping.begin()->segment;
			const auto& last_segment = std::prev(overlapping.end())->segment;
			// If there is only one segment and it is fully contained in the new segment.
			if (&first_segment == &last_segment and first_segment.start <= time_start and time_end <= first_segment.end)
			{
				return std::make_pair(overlapping, false);
			}

			time_start = std::min(first_segment.start, time_start);
			time_end = std::max(last_segment.end, time_end);

			return std::make_pair(overlapping, true);
		}

		return std::nullopt;
	}

	std::optional<tag_timeline::iterator> tag_timeline::prepare_insert(timestamp ts) const
	{
		auto it = find(ts);
		if (it != end())
		{
			return it;
		}

		return std::nullopt;
	}

	void tag_timeline::insert_no_check(segment_id id, tag_segment&& segment)
	{
		auto it = segments_.emplace(lower_bound(segment.start), id, std::move(segment));
		id_map_.try_emplace(id, std::distance(segments_.begin(), it));
		update_id_map(it + 1, segments_.end(), 1);
	}

	tag_timeline::iterator tag_timeline::lower_bound(timestamp ts) const
	{
		return std::lower_bound(segments_.begin(), segments_.end(), ts, [](const auto& obj, const timestamp& ts)
		{
			return obj.segment.start < ts;
		});
	}

	tag_timeline::iterator tag_timeline::lower_bound(iterator begin, timestamp ts) const
	{
		return std::lower_bound(begin, segments_.end(), ts, [](const auto& obj, const timestamp& ts)
		{
			return obj.segment.start < ts;
		});
	}

	tag_timeline::iterator tag_timeline::upper_bound(timestamp ts) const
	{
		return std::upper_bound(segments_.begin(), segments_.end(), ts, [](const timestamp& ts, const auto& obj)
		{
			return ts < obj.segment.start;
		});
	}

	tag_timeline::iterator tag_timeline::upper_bound(iterator begin, timestamp ts) const
	{
		return std::upper_bound(begin, segments_.end(), ts, [](const timestamp& ts, const auto& obj)
		{
			return ts < obj.segment.start;
		});
	}

	void tag_timeline::update_id_map(iterator update_begin, iterator update_end, ptrdiff_t offset)
	{
		for (auto it = update_begin; it != update_end; ++it)
		{
			id_map_.at(it->id) += offset;
		}
	}
}
