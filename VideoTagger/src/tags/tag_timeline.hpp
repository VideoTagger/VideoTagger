#pragma once

#include <string>
#include <utility>
#include <set>
#include <chrono>
#include <unordered_map>
#include <optional>

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
	enum class tag_segment_type
	{
		timestamp,
		segment
	};

	struct tag_segment
	{
		using attribute_instance_container = std::unordered_map<video_id_t, std::unordered_map<std::string, tag_attribute_instance>>;
		static constexpr auto min_segment_size = std::chrono::milliseconds{ 1 };
		static constexpr auto default_segment_size = std::chrono::milliseconds{ 500 };

		timestamp start{};
		timestamp end{};

		mutable attribute_instance_container attributes;

		tag_segment(timestamp time_start, timestamp time_end, const attribute_instance_container& attributes = {});
		tag_segment(timestamp time_point, const attribute_instance_container& attributes = {});

		void set(timestamp time_start, timestamp time_end);
		void set(timestamp time_point);

		std::chrono::nanoseconds duration() const;
		tag_segment_type type() const;
	};

	class tag_timeline
	{
		struct tag_timeline_set_comparator_
		{
			bool operator()(const tag_segment& lhs, const tag_segment& rhs) const
			{
				return lhs.start < rhs.start;
			}
		};

	public:
		using iterator = std::set<tag_segment, tag_timeline_set_comparator_>::iterator;
		using reverse_iterator = std::set<tag_segment, tag_timeline_set_comparator_>::reverse_iterator;

		std::pair<iterator, bool> insert(timestamp time_start, timestamp time_end, const tag_segment::attribute_instance_container& attributes = {});
		std::pair<iterator, bool> insert(timestamp time_point, const tag_segment::attribute_instance_container& attributes = {});
		iterator erase(iterator it);

		//will invalidate it
		std::pair<iterator, bool> replace(iterator it, timestamp new_start, timestamp new_end);
		//will invalidate it
		std::pair<iterator, bool> replace(iterator it, timestamp time_point);

		iterator_range<iterator> find_range(timestamp time_start, timestamp time_end) const;
		iterator find(timestamp time_point) const;

		iterator begin() const;
		reverse_iterator rbegin() const;
		iterator end() const;
		reverse_iterator rend() const;

		size_t size() const;
		bool empty() const;

	private:
		std::set<tag_segment, tag_timeline_set_comparator_> segments_;

		std::optional<std::pair<iterator_range<iterator>, bool>> prepare_insert(timestamp& time_start, timestamp& time_end);
		std::optional<iterator> prepare_insert(timestamp ts);
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
			for (auto& segment : tag_segments)
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
}
