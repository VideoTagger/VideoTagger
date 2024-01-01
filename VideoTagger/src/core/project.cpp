#define _CRT_SECURE_NO_WARNINGS
#include "project.hpp"
#include <vector>
#include <utils/json.hpp>
#include <core/debug.hpp>

static std::chrono::system_clock::time_point to_sys_time(const std::filesystem::file_time_type& ftime)
{
	using namespace std::literals;
	return std::chrono::system_clock::time_point{ ftime.time_since_epoch() - 3234576h };
}

namespace vt
{
    bool project::is_valid() const
	{
		return !path.empty() and !name.empty();
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
		project["name"] = name;
		project["working-dir"] = std::filesystem::relative(working_dir);

		//TODO: Tags
		auto& json_tags = json["tags"];
		json_tags = nlohmann::json::array();
		for (auto& tag : tags)
		{
			json_tags.push_back(nlohmann::json::object({}));
			auto& json_tag_data = json_tags.back();
			json_tag_data["name"] = tag.name;
			json_tag_data["color"] = tag.color;
			auto& json_timestamps = json_tag_data["timestamps"];
			json_timestamps = nlohmann::json::array();
			for (auto& timestamp : tag.timeline)
			{
				json_timestamps.push_back(nlohmann::json::object({ { "start", timestamp.start.count() }, { "end", timestamp.end.count() } }));
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
		return name == other.name and path == other.path and working_dir == other.working_dir;
	}
	
	project project::load_from_file(const std::filesystem::path& filepath)
	{
		project result;
		result.path = filepath;
		if (!std::filesystem::exists(filepath))
		{
			result.name = filepath.stem().string();
		}
		else
		{
			//TODO: Add error checking
			nlohmann::json json = utils::json::load_from_file(filepath);
			const auto& project = json["project"];
			result.name = project["name"];
			result.working_dir = project["working-dir"].get<std::filesystem::path>();

			for (auto& tag_data : json["tags"])
			{
				auto [tag_it, success] = result.tags.insert(tag_data["name"]);
				tag_it->color = tag_data["color"];
				for (auto& timestamp : tag_data["timestamps"])
				{
					tag_it->timeline.insert(timestamp_t{ timestamp["start"] }, timestamp_t{ timestamp["end"] });
				}
			}
		}			
		return result;
	}
}
