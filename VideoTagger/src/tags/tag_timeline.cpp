#include "pch.hpp"
#include "tag_timeline.hpp"
#include <utils/random.hpp>

namespace vt
{
	tag_timeline_insert_result::tag_timeline_insert_result(segment_id preventing_segment)
		: data_{ preventing_segment }
	{
	}

	tag_timeline_insert_result::tag_timeline_insert_result(segment_id inserted_segment, const std::vector<segment_id>& merged_segments)
		: data_{ std::make_pair(inserted_segment, merged_segments) }
	{
	}

	segment_id tag_timeline_insert_result::preventing_segment() const
	{
		return std::get<0>(data_);
	}

	const std::vector<segment_id>& tag_timeline_insert_result::merged_segments() const
	{
		return std::get<1>(data_).second;
	}

	segment_id tag_timeline_insert_result::inserted_segment() const
	{
		return std::get<1>(data_).first;
	}

	bool tag_timeline_insert_result::inserted() const
	{
		return data_.index() == 1;
	}

	tag_timeline_move_result::tag_timeline_move_result(segment_id moved_id, const std::vector<segment_id>& merged_ids, segment_id resulting_id) :
		moved_id_{ moved_id }, merged_ids_{ merged_ids }, resulting_id_{ resulting_id } {}

	segment_id tag_timeline_move_result::moved_segment() const
	{
		return moved_id_;
	}

	const std::vector<segment_id>& tag_timeline_move_result::merged_segments() const
	{
		return merged_ids_;
	}

	segment_id tag_timeline_move_result::resulting_segment() const
	{
		return resulting_id_;
	}

	tag_segment::tag_segment(timestamp time_start, timestamp time_end) 
		: start{ std::min(time_start, time_end) }, end{ std::max(time_start, time_end) } {}

	tag_segment::tag_segment(timestamp ts) 
		: start{ ts }, end{ ts } {}

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

	bool tag_segment::contains(timestamp ts) const
	{
		return start <= ts and ts <= end;
	}

	nlohmann::ordered_json tag_segment::serialize() const
	{
		nlohmann::ordered_json json;
		switch (type())
		{
		case tag_segment_type::timestamp:
			json["timestamp"] = start;
			break;
		case tag_segment_type::segment:
			json["start"] = start;
			json["end"] = end;
			break;
		}
		return json;
	}

	void tag_segment::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("timestamp"))
		{
			start = json["timestamp"];
			end = start;
		}
		else if (json.contains("start") and json.contains("end"))
		{
			start = json["start"];
			end = json["end"];
		}
	}

	tag_timeline_insert_result tag_timeline::insert(timestamp time_start, timestamp time_end, segment_attribute_instances_container&& attributes)
	{
		if (time_start > time_end)
		{
			std::swap(time_start, time_end);
		}

		std::vector<segment_id> merged_segments;
		auto prepare_result = prepare_insert_(time_start, time_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			if (!can_insert)
			{
				return tag_timeline_insert_result(prepare_result->first.begin()->id);
			}

			merged_segments.reserve(overlapping.size());
			for (const auto& [overlapping_id, overlapping_segment] : overlapping)
			{
				merged_segments.push_back(overlapping_id);

				time_start = std::min(time_start, overlapping_segment.start);
				time_end = std::max(time_end, overlapping_segment.end);
			}

			erase(overlapping.begin(), overlapping.end());
		}

		auto id = utils::random::get_mono<segment_id>();
		auto insert_it = segments_.emplace(lower_bound_(time_start), id, time_start, time_end);
		update_id_map_(insert_it + 1, segments_.end(), 1);
		id_map_.try_emplace(id, insert_it - segments_.begin());

		attribute_instances_[id] = std::move(attributes);

		return tag_timeline_insert_result(id, merged_segments);
	}

	tag_timeline_insert_result tag_timeline::insert(timestamp ts, segment_attribute_instances_container&& attributes)
	{
		auto prepare_result = prepare_insert_(ts);
		if (prepare_result.has_value())
		{
			return tag_timeline_insert_result((*prepare_result)->id);
		}

		auto id = utils::random::get_mono<segment_id>();
		auto insert_it = segments_.emplace(lower_bound_(ts), id, ts);
		update_id_map_(insert_it + 1, segments_.end(), 1);
		id_map_.try_emplace(id, insert_it - segments_.begin());

		attribute_instances_[id] = std::move(attributes);

		return tag_timeline_insert_result(id, {});
	}

	tag_timeline_insert_result tag_timeline::insert(tag_segment segment, segment_attribute_instances_container&& attributes)
	{
		if (segment.type() == tag_segment_type::timestamp)
		{
			return insert(segment.start, std::move(attributes));
		}
		else
		{
			return insert(segment.start, segment.end, std::move(attributes));
		}
	}

	bool tag_timeline::erase(segment_id id)
	{
		return erase_(id, true);
	}

	tag_timeline::iterator tag_timeline::erase(iterator it)
	{
		return erase_(it, true);
	}

	tag_timeline::iterator tag_timeline::erase(iterator it_begin, iterator it_end)
	{
		return erase_(it_begin, it_end, true);
	}

	tag_timeline_move_result tag_timeline::move(segment_id id, timestamp new_start, timestamp new_end)
	{
		return move_(id, new_start, new_end, { id });
	}

	tag_timeline_move_result tag_timeline::move(segment_id id, timestamp ts)
	{
		return move_(id, ts, { id });
	}

	tag_timeline_move_result tag_timeline::move_offset(segment_id id, segment_part part, timestamp offset)
	{
		const auto& segment = at(id);
		if (segment.type() == tag_segment_type::timestamp and part == segment_part::both)
		{
			return move(id, segment.start + offset);
		}

		auto move_start = segment.start;
		auto move_end = segment.end;

		if (part & segment_part::left)
		{
			move_start += offset;
		}
		if (part & segment_part::right)
		{
			move_end += offset;
		}

		return move(id, move_start, move_end);
	}

	std::vector<tag_timeline_move_result> tag_timeline::move(const std::vector<segment_move_data>& move_data)
	{
		std::set<segment_id> ignored_segments;
		for (auto& move_data_element : move_data)
		{
			ignored_segments.insert(move_data_element.id);
		}

		std::vector<tag_timeline_move_result> result;
		result.reserve(move_data.size());
		for (auto& move_data_element : move_data)
		{
			result.push_back(move_(move_data_element.id, move_data_element.new_start, move_data_element.new_end, ignored_segments));
			ignored_segments.erase(move_data_element.id);
		}

		return result;
	}

	std::vector<tag_timeline_move_result> tag_timeline::move_offset(const std::vector<segment_move_offset_data>& move_data)
	{
		std::set<segment_id> ignored_segments;
		for (auto& move_data_element : move_data)
		{
			ignored_segments.insert(move_data_element.id);
		}

		std::vector<tag_timeline_move_result> result;
		result.reserve(move_data.size());
		for (auto& move_data_element : move_data)
		{
			const auto& segment = at(move_data_element.id);
			if (segment.type() == tag_segment_type::timestamp and move_data_element.part == segment_part::both)
			{
				result.push_back(move_(move_data_element.id, segment.start + move_data_element.offset, ignored_segments));
				continue;
			}

			auto move_start = segment.start;
			auto move_end = segment.end;

			if (move_data_element.part & segment_part::left)
			{
				move_start += move_data_element.offset;
			}
			if (move_data_element.part & segment_part::right)
			{
				move_end += move_data_element.offset;
			}

			result.push_back(move_(move_data_element.id, move_start, move_end, ignored_segments));
			ignored_segments.erase(move_data_element.id);
		}

		return result;
	}

	std::vector<tag_timeline_move_result> tag_timeline::move_offset(const std::set<segment_id>& ids, segment_part part, timestamp offset)
	{
		if (part == segment_part::none)
		{
			return {};
		}

		std::set<segment_id> ignored_segments = ids;

		std::vector<tag_timeline_move_result> result;
		result.reserve(ids.size());
		for (auto& id : ids)
		{
			const auto& segment = at(id);
			if (segment.type() == tag_segment_type::timestamp and part == segment_part::both)
			{
				result.push_back(move_(id, segment.start + offset, ignored_segments));
				continue;
			}

			auto move_start = segment.start;
			auto move_end = segment.end;

			if (part & segment_part::left)
			{
				move_start += offset;
			}
			if (part & segment_part::right)
			{
				move_end += offset;
			}

			result.push_back(move_(id, move_start, move_end, ignored_segments));
			ignored_segments.erase(id);
		}

		return result;
	}

	iterator_range<tag_timeline::iterator> tag_timeline::find_range(timestamp time_start, timestamp time_end) const
	{
		if (empty())
		{
			return { end(), end() };
		}

		auto range_begin_it = lower_bound_(time_start);
		if (range_begin_it != begin())
		{
			auto prev_it = std::prev(range_begin_it);
			if (prev_it->segment.end >= time_start)
			{
				range_begin_it = prev_it;
			}
		}

		auto range_end_it = upper_bound_(time_end);

		return { range_begin_it, range_end_it };
	}

	tag_timeline::iterator tag_timeline::find(timestamp ts) const
	{
		if (empty())
		{
			return end();
		}

		auto it = lower_bound_(ts);
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

	std::set<segment_id> tag_timeline::find_move_conflicts(segment_id id, segment_part part, timestamp offset) const
	{
		std::set<segment_id> result;
		find_overlapping_(result, id, part, offset, {});
		return result;
	}

	std::set<segment_id> tag_timeline::find_move_conflicts(const std::set<segment_id>& ids, segment_part part, timestamp offset) const
	{
		std::set<segment_id> result;
		for (auto& id : ids)
		{
			find_overlapping_(result, id, part, offset, ids);
		}
		return result;
	}

	const tag_segment& tag_timeline::at(segment_id id) const
	{
		return segments_.at(id_map_.at(id)).segment;
	}

	const segment_attribute_instances_container& tag_timeline::segment_attribute_instances(segment_id id) const
	{
		return attribute_instances_.at(id);
	}

	segment_attribute_instances_container& tag_timeline::segment_attribute_instances(segment_id id)
	{
		return attribute_instances_.at(id);
	}

	const std::unordered_map<segment_id, segment_attribute_instances_container>& tag_timeline::attribute_instances() const
	{
		return attribute_instances_;
	}

	std::unordered_map<segment_id, segment_attribute_instances_container>& tag_timeline::attribute_instances()
	{
		return attribute_instances_;
	}

	size_t tag_timeline::erase_attribute_instances(const std::string& attribute_name, std::function<void(segment_id, video_id_t, impl::attribute_instance*)> on_delete)
	{
		size_t erased = 0;
		for (auto& [id, video_instances] : attribute_instances_)
		{
			for (auto& [video_id, instances] : video_instances)
			{
				for (auto it = instances.begin(); it != instances.end();)
				{
					if ((*it)->attribute_name() == attribute_name)
					{
						erased++;
						if (on_delete != nullptr)
						{
							on_delete(id, video_id, it->get());
						}
						it = instances.erase(it);
						continue;
					}

					++it;
				}
			}
		}

		return erased;
	}

	void tag_timeline::erase_attribute_instances(segment_id id, std::function<void(video_id_t, impl::attribute_instance*)> on_delete)
	{
		for (auto& [video_id, instances] : attribute_instances_.at(id))
		{
			for (auto it = instances.begin(); it != instances.end();)
			{
				if (on_delete != nullptr)
				{
					on_delete(video_id, it->get());
				}
				it = instances.erase(it);
			}
		}
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

	std::optional<std::pair<iterator_range<tag_timeline::iterator>, bool>> tag_timeline::prepare_insert_(timestamp time_start, timestamp time_end) const
	{
		auto overlapping = find_range(time_start, time_end);
		if (overlapping.empty())
		{
			return std::nullopt;
		}

		const auto& first_segment = overlapping.begin()->segment;
		const auto& last_segment = std::prev(overlapping.end())->segment;
		// If there is only one segment and it is fully contained in the new segment.
		if (&first_segment == &last_segment and first_segment.start <= time_start and time_end <= first_segment.end)
		{
			return std::make_pair(overlapping, false);
		}

		return std::make_pair(overlapping, true);
	}

	std::optional<tag_timeline::iterator> tag_timeline::prepare_insert_(timestamp ts) const
	{
		auto it = find(ts);
		if (it != end())
		{
			return it;
		}

		return std::nullopt;
	}

	tag_timeline_move_result tag_timeline::move_(segment_id id, timestamp new_start, timestamp new_end, const std::set<segment_id>& ignored_segments)
	{
		if (new_start > new_end)
		{
			std::swap(new_start, new_end);
		}

		if (new_start == new_end)
		{
			return move_(id, new_start, ignored_segments);
		}

		std::vector<segment_id> merged_segments;

		auto prepare_result = prepare_insert_(new_start, new_end);
		if (prepare_result.has_value())
		{
			auto& overlapping = prepare_result->first;
			bool can_insert = prepare_result->second;

			// If target location is fully contained in the overlapping segment and its not in the ignored segments, just remove the moved segment.
			if (!can_insert and ignored_segments.count(overlapping.begin()->id) == 0)
			{
				auto result_id = prepare_result->first.begin()->id;
				erase(id);
				return tag_timeline_move_result(id, { id }, result_id);
			}

			merged_segments.reserve(overlapping.size());
			for (auto [overlapping_id, overlapping_segment] : overlapping)
			{
				if (ignored_segments.count(overlapping_id) != 0)
				{
					continue;
				}

				new_start = std::min(new_start, overlapping_segment.start);
				new_end = std::max(new_end, overlapping_segment.end);

				merged_segments.push_back(overlapping_id);
			}

			erase_if(overlapping.begin(), overlapping.end(), [&ignored_segments](const auto& obj)
			{
				return ignored_segments.count(obj.id) == 0;
			});
		}

		tag_segment moved_segment = std::move(segments_.at(id_map_.at(id)).segment);
		erase_(id, false);
		moved_segment.set(new_start, new_end);
		insert_no_check_(id, std::move(moved_segment));

		return tag_timeline_move_result(id, merged_segments, id);
	}

	tag_timeline_move_result tag_timeline::move_(segment_id id, timestamp ts, const std::set<segment_id>& ignored_segments)
	{
		auto prepare_result = prepare_insert_(ts);
		if (prepare_result.has_value() and ignored_segments.count(prepare_result.value()->id) == 0)
		{
			auto result_id = (*prepare_result)->id;
			erase(id);

			return tag_timeline_move_result(id, {id}, result_id);
		}

		tag_segment moved_segment = std::move(segments_.at(id_map_.at(id)).segment);
		erase_(id, false);
		moved_segment.set(ts);
		insert_no_check_(id, std::move(moved_segment));

		return tag_timeline_move_result(id, {}, id);
	}

	void tag_timeline::insert_no_check_(segment_id id, tag_segment&& segment)
	{
		auto it = segments_.emplace(lower_bound_(segment.start), id, std::move(segment));
		id_map_.try_emplace(id, std::distance(segments_.begin(), it));
		update_id_map_(it + 1, segments_.end(), 1);
	}

	tag_timeline::iterator tag_timeline::lower_bound_(timestamp ts) const
	{
		return std::lower_bound(segments_.begin(), segments_.end(), ts, [](const auto& obj, const timestamp& ts)
		{
			return obj.segment.start < ts;
		});
	}

	tag_timeline::iterator tag_timeline::lower_bound_(iterator begin, timestamp ts) const
	{
		return std::lower_bound(begin, segments_.end(), ts, [](const auto& obj, const timestamp& ts)
		{
			return obj.segment.start < ts;
		});
	}

	tag_timeline::iterator tag_timeline::upper_bound_(timestamp ts) const
	{
		return std::upper_bound(segments_.begin(), segments_.end(), ts, [](const timestamp& ts, const auto& obj)
		{
			return ts < obj.segment.start;
		});
	}

	tag_timeline::iterator tag_timeline::upper_bound_(iterator begin, timestamp ts) const
	{
		return std::upper_bound(begin, segments_.end(), ts, [](const timestamp& ts, const auto& obj)
		{
			return ts < obj.segment.start;
		});
	}

	void tag_timeline::update_id_map_(iterator update_begin, iterator update_end, ptrdiff_t offset)
	{
		for (auto it = update_begin; it != update_end; ++it)
		{
			id_map_.at(it->id) += offset;
		}
	}

	bool tag_timeline::erase_(segment_id id, bool erase_attributes)
	{
		auto map_it = id_map_.find(id);
		if (map_it == id_map_.end())
		{
			return false;
		}

		auto index = map_it->second;
		auto it = segments_.begin() + index;
		it = segments_.erase(it);
		id_map_.erase(id);
		update_id_map_(it, segments_.end(), -1);

		if (erase_attributes)
		{
			attribute_instances_.erase(id);
		}

		return true;
	}

	tag_timeline::iterator tag_timeline::erase_(iterator it, bool erase_attributes)
	{
		auto id = it->id;
		it = segments_.erase(it);
		id_map_.erase(id);
		update_id_map_(it, segments_.end(), -1);
		if (erase_attributes)
		{
			attribute_instances_.erase(id);
		}

		return it;
	}

	tag_timeline::iterator tag_timeline::erase_(iterator it_begin, iterator it_end, bool erase_attributes)
	{
		for (iterator it = it_begin; it != it_end; ++it)
		{
			id_map_.erase(it->id);
			if (erase_attributes)
			{
				attribute_instances_.erase(it->id);
			}
		}
		auto update_it = segments_.erase(it_begin, it_end);
		update_id_map_(update_it, segments_.end(), -static_cast<int64_t>(std::distance(it_begin, it_end)));

		return update_it;
	}

	void tag_timeline::find_overlapping_(std::set<segment_id>& result, segment_id segment, segment_part part, timestamp offset, const std::set<segment_id>& ignored_segments) const
	{
		const auto& segment_data = at(segment);
		timestamp new_start = segment_data.start;
		timestamp new_end = segment_data.end;
		if (part & segment_part::left)
		{
			new_start += offset;
		}
		if (part & segment_part::right)
		{
			new_end += offset;
		}

		find_overlapping_(result, new_start, new_end, segment, ignored_segments);
	}

	void tag_timeline::find_overlapping_(std::set<segment_id>& result, timestamp start, timestamp end, segment_id ignored_segment, const std::set<segment_id>& ignored_segments) const
	{
		auto overlapping = find_range(start, end);
		for (const auto& [overlapping_id, _] : overlapping)
		{
			if (ignored_segments.count(overlapping_id) != 0 or overlapping_id == ignored_segment)
			{
				continue;
			}

			result.insert(overlapping_id);
		}
	}

	void tag_timeline::find_overlapping_(std::set<segment_id>& result, timestamp start, timestamp end, const std::set<segment_id>& ignored_segments) const
	{
		find_overlapping_(result, start, end, invalid_segment_id, ignored_segments);
	}
}
