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

namespace vt
{
	//TODO: maybe think of something better than "timestamp" for this
	enum class tag_segment_type
	{
		timestamp,
		segment
	};

	struct tag_segment
	{
		static constexpr auto min_segment_size = std::chrono::milliseconds{ 1 };
		static constexpr auto default_segment_size = std::chrono::milliseconds{ 500 };

		timestamp start{};
		timestamp end{};

		tag_segment(timestamp time_start, timestamp time_end);
		tag_segment(timestamp time_point);

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

		std::pair<iterator, bool> insert(timestamp time_start, timestamp time_end);
		std::pair<iterator, bool> insert(timestamp time_point);
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

	inline void from_json(const nlohmann::ordered_json& json, segment_storage& ss)
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
				if (json_tag_segments.contains("timestamp"))
				{
					tag_segments.insert(json_tag_segments["timestamp"].get<timestamp>());
				}
				else if (json_tag_segments.contains("start") and json_tag_segments.contains("end"))
				{
					tag_segments.insert(json_tag_segments["start"].get<timestamp>(), json_tag_segments["end"].get<timestamp>());
				}
			}
		}
	}
}
