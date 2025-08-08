#pragma once

#include <string>
#include <utility>
#include <set>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <vector>

#include <core/debug.hpp>
#include <utils/json.hpp>
#include <utils/time.hpp>
#include <utils/timestamp.hpp>
#include <utils/iterator_range.hpp>
#include "tag.hpp"
#include "tag_storage.hpp"
#include <core/types.hpp>

namespace vt
{
	using segment_id = uint64_t;

	enum class tag_segment_type
	{
		timestamp,
		segment
	};

	///@brief A struct representing a segment or timestamp appearing on the timeline
	struct tag_segment
	{
		using attribute_instance_container = std::unordered_map<video_id_t, std::unordered_map<std::string, tag_attribute_instance>>;
		///@brief Minimum segment length in milliseconds
		static constexpr auto min_segment_size = std::chrono::milliseconds{ 1 };
		///@brief The default segment length in milliseconds used when creating a segment on the timeline
		static constexpr auto default_segment_size = std::chrono::milliseconds{ 500 };

		timestamp start{};
		timestamp end{};

		mutable attribute_instance_container attributes;

		/**
		 * @brief Construct a segment (tag_segment with different start and end). 
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp time_start, timestamp time_end, const attribute_instance_container& attributes = {});

		/**
		 * @brief Construct a timestamp (tag_segment with the same start and end).
		 * 
		 * @param ts Timestamp of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp ts, const attribute_instance_container& attributes = {});

		/**  
		 * @brief Set the start and end time of the segment. If the tag_segment is currently a timestamp, it will become a segment.
		 * If the start time is greater than the end time, they will be swapped.
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 */
		void set(timestamp time_start, timestamp time_end);

		/** 
		 * @brief Set the start and end time of the segment to the same value (make it a timestamp).
		 * @param ts Timestamp of the segment.
		 */
		void set(timestamp ts);

		/** 
		 * @brief Get the length of the segment in nanoseconds.
		 * @return Duration of the segment.
		 */
		std::chrono::milliseconds duration() const;

		/** 
		 * @brief Get the type of the segment (either segment or timestamp).
		 * @return Type of the segment.
		 */
		tag_segment_type type() const;

		/** 
		 * @brief Check if the segment is a timestamp (start and end are the same).
		 * @return True if the segment is a timestamp, false otherwise.
		 */
		bool is_timestamp() const;
	};

	struct segment_with_id
	{
		segment_with_id(segment_id id, timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes = {}) :
			id{ id }, segment{ time_start, time_end, attributes } {}
		segment_with_id(segment_id id, timestamp ts, const tag_segment::attribute_instance_container& attributes = {}) :
			id{ id }, segment{ ts, attributes } {}
		segment_with_id(segment_id id, tag_segment&& segment) :
			id{ id }, segment{ std::move(segment) } {}

		segment_id id;
		tag_segment segment;
	};

	/** 
	 * @brief A container for tag segments, allowing insertion, deletion, and searching of segments by time.
	 * 
	 * Segments are accessed by their unique segment_id, which is generated automatically and remains valid until the segment is erased.
	 * Segments can't be modified directly, but can be moved or erased.
	 * Segment are stored in a sorted order by their start time.
	 */
	class tag_timeline
	{
	public:
		using iterator = std::vector<segment_with_id>::const_iterator;
		using reverse_iterator = std::vector<segment_with_id>::const_reverse_iterator;

		/** 
		 * @brief 
		 */
		std::pair<segment_id, bool> insert(timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes = {});
		std::pair<segment_id, bool> insert(timestamp ts, const tag_segment::attribute_instance_container& attributes = {});
		void erase(segment_id id);
		iterator erase(iterator it);
		iterator erase(iterator it_begin, iterator it_end);
		template<typename Pred>
		iterator erase_if(iterator it_begin, iterator it_end, Pred predicate);

		std::pair<segment_id, bool> replace(segment_id id, timestamp new_start, timestamp new_end);
		std::pair<segment_id, bool> replace(segment_id id, timestamp ts);


		iterator_range<iterator> find_range(timestamp time_start, timestamp time_end) const;
		iterator find(timestamp ts) const;
		const tag_segment& at(segment_id id) const;

		iterator begin() const;
		iterator end() const;
		reverse_iterator rbegin() const;
		reverse_iterator rend() const;

		bool is_id_valid(segment_id id) const;
		size_t size() const;
		bool empty() const;

	private:
		std::vector<segment_with_id> segments_;
		std::unordered_map<segment_id, size_t> id_map_;

		std::optional<std::pair<iterator_range<iterator>, bool>> prepare_insert(timestamp& time_start, timestamp& time_end) const;
		std::optional<iterator> prepare_insert(timestamp ts) const;
		void insert_no_check(segment_id id, tag_segment&& segment);
		iterator lower_bound(timestamp ts) const;
		iterator lower_bound(iterator begin, timestamp ts) const;
		iterator upper_bound(timestamp ts) const;
		iterator upper_bound(iterator begin, timestamp ts) const;
		void update_id_map(iterator update_begin, iterator update_end, ptrdiff_t offset);
	};

	//key: tag name
	using segment_storage = std::unordered_map<std::string, tag_timeline>;

	inline void to_json(nlohmann::ordered_json& json, const tag_segment& segment)
	{
		switch (segment.type())
		{
		case tag_segment_type::timestamp:
		{
			json["timestamp"] = segment.start;
		}
		break;
		case tag_segment_type::segment:
		{
			json["start"] = segment.start;
			json["end"] = segment.end;
		}
		break;
		}

		auto& json_attributes = json["attributes"];
		for (const auto& [vid_id, attr_map] : segment.attributes)
		{
			auto& json_vid_attributes = json_attributes[std::to_string(vid_id)];
			json_vid_attributes = nlohmann::json::array();
			for (const auto& [name, attr] : attr_map)
			{
				if (attr.has_value())
				{
					auto json_attribute = nlohmann::ordered_json::object();
					json_attribute["name"] = name;

					attr.visit([&json_attribute](const auto& value)
					{
						if constexpr (!std::is_same_v<std::monostate, std::remove_cv_t<std::remove_reference_t<decltype(value)>>>)
						{
							json_attribute["value"] = value;
						}
					});
					json_vid_attributes.push_back(json_attribute);
				}
			}
		}
	}

	inline void to_json(nlohmann::ordered_json& json, const segment_storage& ss)
	{
		json = nlohmann::json::array();
		for (auto& [tag_name, tag_segments] : ss)
		{
			nlohmann::ordered_json json_tag_segments_data;
			json_tag_segments_data["tag"] = tag_name;
			auto& json_tag_segments = json_tag_segments_data["tag-segments"];
			json_tag_segments = nlohmann::json::array();
			for (auto& [id, segment] : tag_segments)
			{
				json_tag_segments.push_back(segment);
			}
			json.push_back(json_tag_segments_data);
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, segment_storage& ss, const tag_storage& ts)
	{
		for (const auto& json_group_segments : json)
		{
			if (!json_group_segments.contains("tag"))
			{
				debug::error("Missing tag name");
				continue;
			}
			if (!json_group_segments.contains("tag-segments"))
			{
				debug::error("Missing tag segments");
				continue;
			}

			std::string tag_name = json_group_segments["tag"];
			auto& tag_segments = ss[tag_name];
			for (auto& json_tag_segments : json_group_segments["tag-segments"])
			{
				tag_segment::attribute_instance_container attributes;
				if (json_tag_segments.contains("attributes"))
				{
					for (const auto& [vid_id, vid_attributes] : json_tag_segments["attributes"].items())
					{
						video_id_t vid_id_int{};

						auto [ptr, ec] = std::from_chars(vid_id.c_str(), vid_id.c_str() + vid_id.size(), vid_id_int);
						if (ec != std::errc())
						{
							debug::log("Failed to deserialize video id string: \"{}\"", vid_id);
							continue;
						}

						for (const auto& json_attribute : vid_attributes)
						{
							if (!json_attribute.contains("name") or !json_attribute.contains("value"))
							{
								debug::error("Invalid tag attribute format encountered while deserializing");
								continue;
							}
							auto attribute_name = json_attribute["name"].get<std::string>();
							auto& tag = ts[tag_name];
							if (!tag.attributes.count(attribute_name))
							{
								debug::error("Tag {} doesn't exist, skipping while deserializing", attribute_name);
								continue;
							}

							auto& attribute = attributes[vid_id_int][attribute_name];
							from_json(json_attribute["value"], attribute, tag.attributes.at(attribute_name).type_);
						}
					}
				}

				if (json_tag_segments.contains("timestamp"))
				{
					tag_segments.insert(json_tag_segments["timestamp"].get<timestamp>(), attributes);
				}
				else if (json_tag_segments.contains("start") and json_tag_segments.contains("end"))
				{
					tag_segments.insert(json_tag_segments["start"].get<timestamp>(), json_tag_segments["end"].get<timestamp>(), attributes);
				}
			}
		}
	}

	template<typename Pred>
	inline tag_timeline::iterator tag_timeline::erase_if(iterator it_begin, iterator it_end, Pred predicate)
	{
		ptrdiff_t erased_count = 0;
		auto it = it_begin;
		while (it != it_end)
		{
			if (!predicate(*it))
			{
				++it;
				continue;
			}

			auto end_distance = std::distance(it, it_end);

			id_map_.erase(it->id);
			it = segments_.erase(it);
			it_end = it + (end_distance - 1);
			erased_count++;

			update_id_map(it, it_end, -1);
		}
		update_id_map(it_end, segments_.end(), -erased_count);
		return it;
;
	}
}
