#include "pch.hpp"
#include "project.hpp"
#include <utils/json.hpp>
#include <utils/string.hpp>
#include <core/debug.hpp>
#include <widgets/time_input.hpp>
#include <utils/color.hpp>
#include <utils/hash.hpp>
#include <utils/filesystem.hpp>
#include "app_context.hpp"

static std::chrono::system_clock::time_point to_sys_time(const std::filesystem::file_time_type& ftime)
{
	using namespace std::chrono;
	return time_point_cast<system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + system_clock::now());
}

namespace vt
{
	bool project_info::is_valid() const
	{
		return !path.empty() and !name.empty() and version != 0;
	}

	std::optional<std::tm> project_info::modification_time() const
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

	void project_info::save()
	{
		nlohmann::ordered_json json;
		auto& project = json["project"];
		project["version"] = version;
		project["name"] = name;
		utils::json::write_to_file(json, path);
	}

	bool project_info::operator==(const project_info& other) const
	{
		return (name == other.name) and (path == other.path);
	}

	project_info project_info::load_from_file(const std::filesystem::path& filepath)
	{
		project_info result;
		result.path = std::filesystem::absolute(filepath);
		if (!std::filesystem::exists(filepath))
		{
			result.name = filepath.stem().string();
		}
		else
		{
			auto json = utils::json::load_from_file(filepath);
			if (!json.contains("project"))
			{
				debug::error("Project format is invalid");
				return {};
			}

			const auto& project = json["project"];
			if (!project.contains("version") or !project.contains("name"))
			{
				debug::error("Project format is invalid");
				return {};
			}

			result.version = project["version"];
			result.name = project["name"];
		}
		return result;
	}

	bool project::import_video(const std::filesystem::path& filepath)
	{
		video_id_t video_id = utils::hash::fnv_hash(filepath); //utils::uuid::get()
		
		if (!videos.insert(video_id, filepath)) return false;

		video_group::video_info group_info{};
		group_info.id = video_id;

		auto gid = (ctx_.current_video_group_id == 0) ? utils::uuid::get() : ctx_.current_video_group_id;
		video_groups[gid].insert(group_info);
		videos.get(video_id)->update_data(ctx_.renderer);

		return true;
	}
	
	void project::save() const
	{
		//TODO: Add error checking

		nlohmann::ordered_json json;
		auto& project = json["project"];
		project["version"] = version;
		project["name"] = name;

		auto& json_tags = json["tags"];
		json_tags = nlohmann::json::array();
		for (auto& tag : tags)
		{
			nlohmann::ordered_json json_tag_data;
			json_tag_data["name"] = tag.name;
			json_tag_data["color"] = utils::color::to_string(tag.color);

			auto segments_it = segments.find(tag.name);
			if (segments_it != segments.end())
			{
				auto& tag_segments = segments_it->second;
				auto& json_segments = json_tag_data["timestamps"];
				json_segments = nlohmann::json::array();
				for (const auto& segment : tag_segments)
				{
					auto segment_json = nlohmann::ordered_json::object();
					switch (segment.type())
					{
					case tag_segment_type::point:
					{
						segment_json["point"] = utils::time::time_to_string(segment.start.seconds_total.count());
					}
					break;
					default:
					{
						segment_json["start"] = utils::time::time_to_string(segment.start.seconds_total.count());
						segment_json["end"] = utils::time::time_to_string(segment.end.seconds_total.count());
					}
					break;
					}
					json_segments.push_back(segment_json);
				}
			}
			json_tags.push_back(json_tag_data);
		}

		auto& json_videos = json["videos"];
		json_videos = nlohmann::json::array();
		for (auto& [id, metadata] : videos)
		{
			nlohmann::ordered_json vid;
			vid["id"] = id;

			std::string vid_path = utils::filesystem::normalize(std::filesystem::relative(metadata.path));

			vid["path"] = vid_path;
			json_videos.push_back(vid);
		}

		auto& json_groups = json["groups"];
		json_groups = nlohmann::json::array();
		for (auto& [id, group] : video_groups)
		{
			nlohmann::ordered_json json_group;
			json_group["id"] = id;
			auto& group_videos = json_group["videos"];
			group_videos = nlohmann::json::array();
			for (auto& video : group)
			{
				nlohmann::ordered_json group_video;
				group_video["id"] = video.id;
				group_video["offset"] = utils::time::time_to_string(video.offset.count());				
				group_videos.push_back(group_video);
			}
			json_groups.push_back(json_group);
		}
		json["keybinds"] = keybinds;

		//TODO: This probably shouldnt be done
		/*
		auto parent = path.parent_path();
		if (!parent.empty())
		{
			std::filesystem::create_directories(parent);
		}
		*/
		utils::json::write_to_file(json, path);
	}

	void project::save_as(const std::filesystem::path& filepath)
	{
		path = filepath;
		save();
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
			auto json = utils::json::load_from_file(filepath);
			if (!json.contains("project"))
			{
				debug::error("Project forrmat is invalid");
				return {};
			}

			const auto& project = json["project"];
			if (!project.contains("version") or !project.contains("name"))
			{
				debug::error("Project format is invalid");
				return {};
			}

			result.version = project["version"];
			result.name = project["name"];

			if (result.version < project::current_version)
			{
				//TODO: Run project updater/error
			}

			if (json.contains("tags"))
			{
				for (auto& tag_data : json["tags"])
				{
					auto [tag_it, success] = result.tags.insert(tag_data["name"]);
					auto col_str = tag_data["color"].get<std::string>();
					uint32_t color{};
					if (utils::color::parse_string(col_str, color))
					{
						tag_it->color = color;
					}

					for (auto& segment : tag_data["timestamps"])
					{
						if (segment.contains("point"))
						{
							auto point = utils::time::parse_time_to_sec(segment["point"]);
							result.segments[tag_it->name].insert(vt::timestamp{ point }, vt::timestamp{ point });
						}
						else if (segment.contains("start") and segment.contains("end"))
						{
							auto start = utils::time::parse_time_to_sec(segment["start"]);
							auto end = utils::time::parse_time_to_sec(segment["end"]);
							result.segments[tag_it->name].insert(vt::timestamp{ start }, vt::timestamp{ end });
						}
					}
				}
			}

			if (!json.contains("groups") or !json["groups"].is_array())
			{
				debug::error("Project's video groups format was invalid");
			}
			else
			{
				const auto& json_groups = json["groups"];
				for (auto& group : json_groups)
				{
					video_group vgroup;
					if (!group.contains("id"))
					{
						debug::error("Project's video group doesn't contain id, skipping...");
						continue;
					}

					video_group_id_t id = group["id"];
					if (!group.contains("videos") or !json["videos"].is_array())
					{
						debug::error("Project's video group's videos format was invalid, skipping...");
						continue;
					}

					const auto& group_videos = group["videos"];
					for (const auto& group_video : group_videos)
					{
						if (!group_video.contains("id") or !group_video.contains("offset"))
						{
							debug::error("Video's format was invalid, skipping...");
							continue;
						}

						video_group::video_info vinfo;
						vinfo.id = group_video["id"];
						vinfo.offset = (decltype(vinfo.offset))utils::time::parse_time_to_sec(group_video["offset"]);
						vgroup.insert(vinfo);
					}
					result.video_groups.insert({ id, vgroup });
				}
			}

			if (json.contains("keybinds"))
			{
				result.keybinds = json["keybinds"];
			}
		}			
		return result;
	}
}
