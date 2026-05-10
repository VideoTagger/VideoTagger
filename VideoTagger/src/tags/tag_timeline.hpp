#pragma once

#include <string>
#include <set>
#include <utility>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <vector>
#include <variant>
#include <charconv>

#include <core/debug.hpp>
#include <utils/json.hpp>
#include <utils/time.hpp>
#include <utils/timestamp.hpp>
#include <utils/iterator_range.hpp>
#include "tag.hpp"
#include "tag_storage.hpp"
#include <core/types.hpp>
#include <attributes/impl/attribute_instance.hpp>
#include <attributes/impl/attribute.hpp>

namespace vt
{
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
	inline constexpr bool operator&(segment_part lhs, segment_part rhs)
	{
		return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
	}

	///@brief A struct representing a segment or timestamp appearing on the timeline
	struct tag_segment : impl::serializable
	{
		///@brief Minimum segment length in milliseconds
		static constexpr auto min_segment_size = std::chrono::milliseconds{ 1 };
		///@brief The default segment length in milliseconds used when creating a segment on the timeline
		static constexpr auto default_segment_size = std::chrono::milliseconds{ 500 };

		timestamp start{};
		timestamp end{};

		tag_segment() = default;

		/**
		 * @brief Construct a segment (tag_segment with different start and end)
		 * 
		 * @param time_start Start time of the segment.
		 * @param time_end End time of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp time_start, timestamp time_end);

		/**
		 * @brief Construct a timestamp (tag_segment with the same start and end)
		 * 
		 * @param ts Timestamp of the segment.
		 * @param attributes Optional attributes associated with the segment.
		 */
		tag_segment(timestamp ts);

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

		///@return Whether the given timestamp is within the bound of the segment
		bool contains(timestamp ts) const;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
	
	using segment_attribute_instances_container = std::unordered_map<video_id_t, std::vector<std::unique_ptr<impl::attribute_instance>>>;

	struct segment_with_id
	{
		segment_with_id(segment_id id, timestamp time_start, timestamp time_end) :
			id{ id }, segment{ time_start, time_end } {}
		segment_with_id(segment_id id, timestamp ts) :
			id{ id }, segment{ ts } {}
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
		tag_timeline(const tag_timeline&) = delete;
		tag_timeline(tag_timeline&&) = default;

		tag_timeline& operator=(const tag_timeline&) = delete;
		tag_timeline& operator=(tag_timeline&&) = default;

	private:
		std::vector<segment_with_id> segments_;
		std::unordered_map<segment_id, size_t> id_map_;
		std::unordered_map<segment_id, segment_attribute_instances_container> attribute_instances_;

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
		tag_timeline_insert_result insert(timestamp time_start, timestamp time_end, segment_attribute_instances_container&& attributes = {});

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
		tag_timeline_insert_result insert(timestamp ts, segment_attribute_instances_container&& attributes = {});

		/**
		 * @brief Insert a new segment
		 *
		 * Invalidates all iterators and references to the segments.
		 * If the inserted segment overlaps with an existing segments it won't be inserted.
		 *
		 * @param segment The segment to insert.
		 * @param attributes Optional attributes associated with the segment.
		 * @return Struct containing information about the result of the insert operation.
		 */
		tag_timeline_insert_result insert(tag_segment segment, segment_attribute_instances_container&& attributes = {});

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
		 * @brief Find all segments that would cause conflict during a move operation.
		 *
		 * @param id ID of segment to move.
		 * @param part Which part of the segment to move. Can't be segment_part::none
		 * @param offset By how much to move the segment relative to its current positions.
		 * @return Set of IDs of the segments that would cause conflicts during the move operation.
		 */
		std::set<segment_id> find_move_conflicts(segment_id id, segment_part part, timestamp offset) const;

		/**
		 * @brief Find all segments that would cause conflict during a move operation.
		 * 
		 * @param ids IDs of segments to move.
		 * @param part Which part of the segments to move. Can't be segment_part::none
		 * @param offset By how much to move the segments relative to their current positions.
		 * @return Set of IDs of the segments that would cause conflicts during the move operation.
		 */
		std::set<segment_id> find_move_conflicts(const std::set<segment_id>& ids, segment_part part, timestamp offset) const;

		/**
		 * @brief Get a segment by its id
		 * 
		 * @param id ID of the segment to get.
		 * @return A const reference to the segment with the given id.
		 */
		const tag_segment& at(segment_id id) const;

		const segment_attribute_instances_container& segment_attribute_instances(segment_id id) const;
		segment_attribute_instances_container& segment_attribute_instances(segment_id id);

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
		
		bool erase_(segment_id id, bool erase_attributes);
		iterator erase_(iterator it, bool erase_attributes);
		iterator erase_(iterator it_begin, iterator it_end, bool erase_attributes);
		template<typename Pred>
		iterator erase_if_(iterator it_begin, iterator it_end, Pred predicate, bool erase_attributes);

		void find_overlapping_(std::set<segment_id>& result, segment_id segment, segment_part part, timestamp offset, const std::set<segment_id>& ignored_segments) const;
		void find_overlapping_(std::set<segment_id>& result, timestamp start, timestamp end, segment_id ignored_segment, const std::set<segment_id>& ignored_segments) const;
		void find_overlapping_(std::set<segment_id>& result, timestamp start, timestamp end, const std::set<segment_id>& ignored_segments) const;
	};

	//key: tag name
	using segment_storage = std::unordered_map<std::string, tag_timeline>;

	/**
	 * @brief Serialize a segment_storage to JSON
	 * 
	 * @param json JSON object to serialize to.
	 * @param ss The segment_storage to serialize.
	 */
	inline void to_json(nlohmann::ordered_json& json, const segment_storage& ss)
	{
		json = nlohmann::ordered_json::array();
		for (auto& [tag_name, tag_segments] : ss)
		{
			nlohmann::ordered_json json_tag_segments_data;
			json_tag_segments_data["tag"] = tag_name;
			auto& json_tag_segments = json_tag_segments_data["tag-segments"];
			json_tag_segments = nlohmann::ordered_json::array();
			for (auto& [id, segment] : tag_segments)
			{
				auto segment_json = segment.serialize();

				const auto& segment_attr_instances = tag_segments.segment_attribute_instances(id);
				auto& segment_attributes_json = segment_json["attributes"];
				for (auto& [video_id, attr_instances] : segment_attr_instances)
				{
					auto& video_attr_instances_json = segment_attributes_json[std::to_string(video_id)];
					video_attr_instances_json = nlohmann::ordered_json::array();
					for (auto& attr_instance : attr_instances)
					{
						if (attr_instance == nullptr) continue;

						video_attr_instances_json.push_back(impl::attribute::serialize_instance(*attr_instance));
					}
				}
				json_tag_segments.push_back(segment_json);
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
			if (!ts.contains(tag_name))
			{
				debug::error("Tag {} doesn't exist, skipping segments deserializing", tag_name);
				continue;
			}

			auto& tag = ts.at(tag_name);
			auto& tag_segments = ss[tag_name];
			for (const auto& json_tag_segment : json_group_segments["tag-segments"])
			{
				auto segment = json_tag_segment.get<tag_segment>();

				segment_attribute_instances_container attributes;
				if (json_tag_segment.contains("attributes"))
				{
					for (const auto& [vid_id, vid_attributes] : json_tag_segment["attributes"].items())
					{
						video_id_t vid_id_int{};

						auto [ptr, ec] = std::from_chars(vid_id.c_str(), vid_id.c_str() + vid_id.size(), vid_id_int);
						if (ec != std::errc())
						{
							debug::log("Failed to deserialize video id string: \"{}\"", vid_id);
							continue;
						}

						auto& video_attribute_instances = attributes[vid_id_int];

						for (const auto& json_attribute_instance : vid_attributes)
						{
							video_attribute_instances.push_back(tag.deserialize_attribute_instance(json_attribute_instance));
						}
					}
				}

				tag_segments.insert(segment, std::move(attributes));
			}
		}
	}

	template<typename Pred>
	inline tag_timeline::iterator tag_timeline::erase_if(iterator it_begin, iterator it_end, Pred predicate)
	{
		return erase_if_(it_begin, it_end, predicate, true);
	}

	template<typename Pred>
	inline tag_timeline::iterator tag_timeline::erase_if_(iterator it_begin, iterator it_end, Pred predicate, bool erase_attributes)
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
			if (erase_attributes)
			{
				attribute_instances_.erase(it->id);
			}
			it = segments_.erase(it);
			it_end = it + (end_distance - 1);
			erased_count++;

			update_id_map_(it, it_end, -1);
		}
		update_id_map_(it_end, segments_.end(), -erased_count);
		return it;
	}


	inline auto segment_id_map_find(const segment_id_map& map, const std::string& tag_name, segment_id id) ->
		std::pair<segment_id_map::const_iterator, segment_id_map::mapped_type::const_iterator>
	{
		auto it = map.find(tag_name);
		if (it == map.end())
		{
			return { map.end(), {} };
		}

		const auto& id_set = it->second;
		auto id_it = id_set.find(id);
		if (id_it == id_set.end())
		{
			return { it, id_set.end() };
		}

		return { it, id_it };
	}

	inline auto segment_id_map_find(segment_id_map& map, const std::string& tag_name, segment_id id) ->
		std::pair<segment_id_map::iterator, segment_id_map::mapped_type::iterator>
	{
		auto it = map.find(tag_name);
		if (it == map.end())
		{
			return { map.end(), {} };
		}

		const auto& id_set = it->second;
		auto id_it = id_set.find(id);
		if (id_it == id_set.end())
		{
			return { it, id_set.end() };
		}

		return { it, id_it };
	}

	inline bool segment_id_map_contains(const segment_id_map& map, const std::string& tag_name, segment_id id)
	{
		auto [map_it, id_it] = segment_id_map_find(map, tag_name, id);
		return map_it != map.end() and id_it != map_it->second.end();
	}

	inline bool segment_id_map_erase(segment_id_map& map, const std::string& tag_name, segment_id id)
	{
		auto [tags_it, segments_it] = segment_id_map_find(map, tag_name, id);
		if (tags_it == map.end() or segments_it == tags_it->second.end())
		{
			return false;
		}

		tags_it->second.erase(segments_it);
		if (tags_it->second.empty())
		{
			map.erase(tags_it);
		}
		return true;
	}

	inline std::pair<timestamp, timestamp> min_max_segment_timestamps(const segment_storage& storage, const segment_id_map& segments)
	{
		timestamp min_timestamp = timestamp::max();
		timestamp max_timestamp = timestamp::min();
		for (auto& [tag, segment_ids] : segments)
		{
			auto storage_it = storage.find(tag);
			if (storage_it == storage.end()) continue;

			auto& tag_segments = storage_it->second;
			for (auto& segment_id : segment_ids)
			{
				if (!tag_segments.is_id_valid(segment_id)) continue;

				auto& segment = tag_segments.at(segment_id);
				min_timestamp = std::min(min_timestamp, segment.start);
				max_timestamp = std::max(max_timestamp, segment.end);
			}
		}

		return { min_timestamp, max_timestamp };
	}
}
