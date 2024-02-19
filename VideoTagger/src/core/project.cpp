#define _CRT_SECURE_NO_WARNINGS
#include "project.hpp"
#include <vector>
#include <utils/json.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>
#include <widgets/time_input.hpp>

static std::chrono::system_clock::time_point to_sys_time(const std::filesystem::file_time_type& ftime)
{
	using namespace std::literals;
	return std::chrono::system_clock::time_point{ ftime.time_since_epoch() - 3234576h };
}

namespace vt
{
    bool project::is_valid() const
	{
		return !path.empty() and !name.empty() and version != 0;
	}

	std::optional<std::tm> project::modification_time() const
	{
		std::optional<std::tm> result;
		if (path.empty() or !std::filesystem::exists(path)) return std::nullopt;
		std::filesystem::file_time_type mod_time = std::filesystem::last_write_time(path);

		if (mod_time != std::filesystem::file_time_type{})
		{
			auto time = to_sys_time(mod_time);
			auto tt = std::chrono::system_clock::to_time_t(time);
			result = *std::localtime(&tt);
		}
		return result;
	}
	
	void project::save() const
	{
		//TODO: Add error checking

		nlohmann::ordered_json json;
		auto& project = json["project"];
		project["version"] = version;
		project["name"] = name;

		//TODO: Tags
		auto& json_tags = json["tags"];
		json_tags = nlohmann::json::array();
		for (auto& tag : tags)
		{
			json_tags.push_back(nlohmann::json::value_t::object);
			auto& json_tag_data = json_tags.back();
			json_tag_data["name"] = tag.name;
			decltype(tag.color) rgb = ((tag.color & 0x0000FF) << 16) + (tag.color & 0x00FF00) + ((tag.color & 0xFF0000) >> 16);
			json_tag_data["color"] = "#" + utils::string::to_hex(rgb, (sizeof(decltype(rgb)) << 1) - 2);
			auto& json_timestamps = json_tag_data["timestamps"];
			json_timestamps = nlohmann::json::array();
			for (const auto& timestamp : tag.timeline)
			{
				auto timestamp_json = nlohmann::ordered_json::object();
				switch (timestamp.type())
				{
					case tag_timestamp_type::point:
					{
						timestamp_json["point"] = utils::time::time_to_string(timestamp.start.seconds_total.count());
					}
					break;
					default:
					{
						timestamp_json["start"] = utils::time::time_to_string(timestamp.start.seconds_total.count());
						timestamp_json["end"] = utils::time::time_to_string(timestamp.end.seconds_total.count());
					}
					break;
				}
				json_timestamps.push_back(timestamp_json);
			}
		}

		//TODO: Keybinds
		auto& keybinds = json["keybinds"];
		keybinds = nlohmann::json::array();
		auto parent = path.parent_path();
		if (!parent.empty())
		{
			std::filesystem::create_directories(parent);
		}
		utils::json::write_to_file(json, path);
	}

	bool project::operator==(const project& other) const
	{
		return (name == other.name) and (path == other.path);
	}
	
	project project::load_from_file(const std::filesystem::path& filepath)
	{
		project result;
		result.path = std::filesystem::absolute(filepath);
		if (!std::filesystem::exists(filepath))
		{
			result.name = filepath.stem().string();
		}
		else
		{
			//TODO: Add error checking
			nlohmann::json json = utils::json::load_from_file(filepath);
			const auto& project = json["project"];
			result.version = project["version"];
			result.name = project["name"];

			for (auto& tag_data : json["tags"])
			{
				auto [tag_it, success] = result.tags.insert(tag_data["name"]);
				auto col_str = tag_data["color"].get<std::string>();
				if (col_str.size() == 7 and col_str[0] == '#')
				{
					auto rgb = std::stoul(col_str.substr(1), nullptr, 16);
					auto abgr = (0xFF << 24) + ((rgb & 0x0000FF) << 16) + (rgb & 0x00FF00) + ((rgb & 0xFF0000) >> 16);
					tag_it->color = abgr;
				}
				for (auto& timestamp : tag_data["timestamps"])
				{
					if (timestamp.contains("point"))
					{
						auto point = utils::time::parse_time_to_sec(timestamp["point"]);
						tag_it->timeline.insert(vt::timestamp{ point }, vt::timestamp{ point });
					}
					else if (timestamp.contains("start") and timestamp.contains("end"))
					{
						auto start = utils::time::parse_time_to_sec(timestamp["start"]);
						auto end = utils::time::parse_time_to_sec(timestamp["end"]);
						tag_it->timeline.insert(vt::timestamp{ start }, vt::timestamp{ end });
					}
				}
			}
		}			
		return result;
	}
}
