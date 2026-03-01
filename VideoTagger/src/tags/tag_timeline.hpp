#pragma once

#include <string>
#include <set>
#include <utility>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <vector>
#include <variant>

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
	using segment_id_map = std::unordered_map<std::string, std::set<segment_id>>;
	inline constexpr auto invalid_segment_id = segment_id{ 0 };

	///@brief Enum representing the type of a segment
	enum class tag_segment_type
	{
		///@brief A point in time (i.e., start == end)
		timestamp,
		///@brief A period of time (i.e., start != end)
		segment
	};

	///@brief Enum representing which part of the segment is being grabbed
	enum class segment_part : uint8_t
	{
		none = 0b00,
		left = 0b01,
		right = 0b10,
		both = left | right,
	};

	///@brief Struct representing data needed to move a segment to a new location
	struct segment_move_data
	{
		///@brief ID of the segment to move
		segment_id id;
		///@brief New start time of the segment
		timestamp new_start;
		///@brief New end time of the segment
		timestamp new_end;
	};

	///@brief Struct representing data needed to move a segment by an offset
	struct segment_move_offset_data
	{
		///@brief ID of the segment to move
		segment_id id;
		///@brief Which part of the segment to move
		segment_part part;
		///@brief By how much to move the segment relative to its current position
		timestamp offset;
	};

	/**
	 * @brief Result of an insertion in the tag_timeline class
	 * 
	 * It can hold one of two values:
	 *  -# ID of the segment which prevented the insertion when inserted() returns false
	 *  -# ID of the inserted segment and a list of the IDs of the merged segments when inserted() returns true
	 */
	class tag_timeline_insert_result
	{
	public:
		explicit tag_timeline_insert_result(segment_id preventing_segment);
		tag_timeline_insert_result(segment_id inserted_segment, const std::vector<segment_id>& merged_segments);

		///@return ID of the segment which prevented insertion.
		segment_id preventing_segment() const;

		///@return Vector of segment IDs which were merged.
		const std::vector<segment_id>& merged_segments() const;

		///@return ID of the inserted segment.
		segment_id inserted_segment() const;

		///@return True if the segment was inserted, false otherwise.
		bool inserted() const;

	private:
		std::variant<segment_id, std::pair<segment_id, std::vector<segment_id>>> data_;
	};

	class tag_timeline_move_result
	{
	public:
		tag_timeline_move_result(segment_id moved_id, const std::vector<segment_id>& merged_ids, segment_id resulting_id);

		///@return ID of the moved segment.
		segment_id moved_segment() const;

		///@return Vector of segment IDs that were merged.
		const std::vector<segment_id>& merged_segments() const;

		///@return ID of the resulting segment.
		segment_id resulting_segment() const;

	private:
		segment_id moved_id_;
		std::vector<segment_id> merged_ids_;
		segment_id resulting_id_;
	};

	/**
	 * @brief Check if lhs contains flag rhs
	 *
	 * @param lhs Value to check.
	 * @param rhs Flag to check for.
	 * @return true if lhs contains flag rhs, false otherwise.
	 */
	inline constexpr bool operator& (segment_part lhs, segment_part rhs)
	{
		return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
	}

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
		 * @brief Construct a segment (tag_segment with different start and end)
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp time_start, timestamp time_end, const attribute_instance_container& attributes = {});

		/**
		 * @brief Construct a timestamp (tag_segment with the same start and end)
		 * 
		 * @param ts Timestamp of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp ts, const attribute_instance_container& attributes = {});

		/**  
		 * @brief Set the start and end time of the segment. If the tag_segment is currently a timestamp, it will become a segment
		 * 
		 * If the start time is greater than the end time, they will be swapped.
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 */
		void set(timestamp time_start, timestamp time_end);

		/** 
		 * @brief Set the start and end time of the segment to the same value (make it a timestamp)
		 * @param ts Timestamp of the segment.
		 */
		void set(timestamp ts);

		///@return Duration of the segment in nanoseconds.
		std::chrono::milliseconds duration() const;

		///@return Type of the segment.
		tag_segment_type type() const;

		///@return True if the segment is a timestamp, false otherwise.
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
	 * @brief A container for tag segments, allowing insertion, deletion, and searching of segments by time
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

		tag_timeline() = default;

	private:
		std::vector<segment_with_id> segments_;
		std::unordered_map<segment_id, size_t> id_map_;

	public:
		/** 
		 * @brief Insert a new segment
		 * 
		 * Invalidates all iterators and references to the segments.
		 * If the inserted segment overlaps with an existing segments the existing segments will be erased and the inserted segment will be merged with them.
		 * If the inserted segment is fully contained in an existing segment, it won't be inserted.
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 * @return Struct containing information about the result of the insert operation.
		 */
		tag_timeline_insert_result insert(timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes = {});

		/**
		 * @brief Insert a new timestamp segment
		 * 
		 * Invalidates all iterators and references to the segments.
		 * If the inserted segment overlaps with an existing segments it won't be inserted.
		 * 
		 * @param ts Position of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 * @return Struct containing information about the result of the insert operation.
		 */
		tag_timeline_insert_result insert(timestamp ts, const tag_segment::attribute_instance_container& attributes = {});

		/**
		 * @brief Erase a segment by its id
		 * 
		 * Invalidates all iterators and references to the segments after the erased segment.
		 * 
		 * @param id ID of the segment to erase.
		 * @return True if the segment was erased, false if no segment with the given ID exists.
		 */
		bool erase(segment_id id);

		/**
		 * @brief Erase a segment by its iterator
		 * 
		 * Invalidates all iterators and references to the segments after the erased segment.
		 * 
		 * @param it Iterator to the segment to erase.
		 * @return Iterator to the next element after the erased one.
		 */
		iterator erase(iterator it);

		/**
		 * @brief Erase a range of segments
		 * 
		 * Invalidates all iterators and references to the segments after the erased range.
		 * 
		 * @param it_begin Iterator to the first segment to erase.
		 * @param it_end Iterator to the last segment to erase (exclusive).
		 * @return Iterator to the next element after the erased range.
		 */
		iterator erase(iterator it_begin, iterator it_end);

		/**
		 * @brief Erase segments that match a predicate
		 * 
		 * Invalidates all iterators and references to the segments after and within the given range.
		 * Unless no segments are erased, in which case the iterators remain valid.
		 * 
		 * @tparam Pred Predicate type.
		 * @param it_begin Iterator to the first segment to consider for erasure.
		 * @param it_end Iterator to the last segment to consider for erasure (exclusive).
		 * @param predicate Predicate which takes a const reference to segment_with_id and returns true if the segment should be erased. 
		 * @return Iterator to the next element after the erased range.
		 */
		template<typename Pred>
		iterator erase_if(iterator it_begin, iterator it_end, Pred predicate);

		/**
		 * @brief Moves a segment to the given location, merging it with existing segments if necessary
		 * 
		 * Invalidates all iterators and references to the segments after the moved segment.
		 * If the new location overlaps with existing segments, they will be merged.
		 * If the new location is fully contained in an existing segment, the moved segment will be erased.
		 * 
		 * @param id ID of the segment to move.
		 * @param new_start New start time of the segment.
		 * @param new_end New end time of the segment.
		 * @return Struct containing information about the result of the move operation.
		 */
		tag_timeline_move_result move(segment_id id, timestamp new_start, timestamp new_end);

		/**
		 * @brief Moves a segment to the given location, merging it with existing segments if necessary
		 * 
		 * Invalidates all iterators and references to the segments after the moved segment.
		 * If the new location overlaps with existing segments, the moved segment will be erased.
		 * 
		 * @param id ID of the segment to move.
		 * @param ts New location of the segment.
		 * @return Struct containing information about the result of the move operation.
		 */
		tag_timeline_move_result move(segment_id id, timestamp ts);

		/**
		 * @brief Moves the specified part of a segment by the given offset, merging it with existing segments if necessary
		 *
		 * Invalidates all iterators and references to the segments after the moved segment.
		 * If the segment is a timestamp, the part parameter is ignored.
		 *
		 * @param id ID of the segment to move.
		 * @param part The part of the segment to move. Can't be segment_part::none.
		 * @param offset By how much to move the segment relative to its current position.
		 * @return Struct containing information about the result of the move operation.
		 */
		tag_timeline_move_result move_offset(segment_id id, segment_part part, timestamp offset);

		//TODO: could be a template and take iterators or iterator_range instead of vector
		/**
		 * @brief Moves multiple segments to the given locations, merging them with existing segments if necessary
		 *
		 * Invalidates all iterators and references to the segments after the moved segment.
		 * If the new location overlaps with existing segments, they will be merged.
		 * If the new location is fully contained in an existing segment, the moved segment will be erased.
		 *
		 * @param move_data Vector containing which segments to move and where to move them.
		 * @return Vector of structs containing information about the result of the move operation.
		 */
		std::vector<tag_timeline_move_result> move(const std::vector<segment_move_data>& move_data);

		//TODO: could be a template and take iterators or iterator_range instead of vector
		/**
		 * @brief Moves the specified parts of segments by the given offset, merging them with existing segments if necessary
		 *
		 * Invalidates all iterators and references to the segments after the moved segments.
		 * If the segment is a timestamp, the part parameter is ignored.
		 *
		 * @param move_data Vector containing which segments to move and by what offset to move them.
		 * @return Vector of structs containing information about the result of the move operation.
		 */
		std::vector<tag_timeline_move_result> move_offset(const std::vector<segment_move_offset_data>& move_data);

		//TODO: could be a template and take iterators or iterator_range instead of set
		/**
		 * @brief Moves the specified parts of segments by the given offset, merging them with existing segments if necessary
		 *
		 * Invalidates all iterators and references to the segments after the moved segments.
		 * If the segment is a timestamp, the part parameter is ignored.
		 *
		 * @param ids IDs of segments to move
		 * @param part Which part of the segments to move. Can't be segment_part::none.
		 * @param offset By how much to move the segments relative to their current positions.
		 * @return Vector of structs containing information about the result of the move operation.
		 */
		std::vector<tag_timeline_move_result> move_offset(const std::set<segment_id>& ids, segment_part part, timestamp offset);

		/**
		 * @brief Find all segments that overlap with the given range
		 * 
		 * @param time_start Start time of the range.
		 * @param time_end End time of the range.
		 * @return A range of the segments that overlap with the given range. If no segments overlap, the range will be empty (will contain two end iterators).
		 */
		iterator_range<iterator> find_range(timestamp time_start, timestamp time_end) const;

		/**
		 * @brief Find a segment that overlaps with the given timestamp
		 * 
		 * @param ts Timestamp to search for.
		 * @return An iterator to the segment that overlaps with the given timestamp. If no segment overlaps, the end iterator will be returned.
		 */
		iterator find(timestamp ts) const;

		/**
		 * @brief Get a segment by its id
		 * 
		 * @param id ID of the segment to get.
		 * @return A const reference to the segment with the given id.
		 */
		const tag_segment& at(segment_id id) const;

		iterator begin() const;
		iterator end() const;
		reverse_iterator rbegin() const;
		reverse_iterator rend() const;

		/**
		 * @brief Check if the given id is valid (exists in the timeline)
		 * 
		 * @param id ID to check.
		 * @return True if the id is valid, false otherwise.
		 */
		bool is_id_valid(segment_id id) const;

		///@return The number of segments in the timeline.
		size_t size() const;

		///@return True if the timeline is empty, false otherwise.
		bool empty() const;

	private:
		std::optional<std::pair<iterator_range<iterator>, bool>> prepare_insert_(timestamp time_start, timestamp time_end) const;
		std::optional<iterator> prepare_insert_(timestamp ts) const;
		tag_timeline_move_result move_(segment_id id, timestamp new_start, timestamp new_end, const std::set<segment_id>& ignored_segments);
		tag_timeline_move_result move_(segment_id id, timestamp ts, const std::set<segment_id>& ignored_segments);
		void insert_no_check_(segment_id id, tag_segment&& segment);
		iterator lower_bound_(timestamp ts) const;
		iterator lower_bound_(iterator begin, timestamp ts) const;
		iterator upper_bound_(timestamp ts) const;
		iterator upper_bound_(iterator begin, timestamp ts) const;
		void update_id_map_(iterator update_begin, iterator update_end, ptrdiff_t offset);
	};

	//key: tag name
	using segment_storage = std::unordered_map<std::string, tag_timeline>;

	/**
	 * @brief Serialize a tag_segment to JSON
	 * 
	 * @param json JSON object to serialize to.
	 * @param segment The tag_segment to serialize.
	 */
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

	/**
	 * @brief Serialize a segment_storage to JSON
	 * 
	 * @param json JSON object to serialize to.
	 * @param ss The segment_storage to serialize.
	 */
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

			update_id_map_(it, it_end, -1);
		}
		update_id_map_(it_end, segments_.end(), -erased_count);
		return it;
;
	}
}
