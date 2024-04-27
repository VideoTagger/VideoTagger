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

	std::future<project_import_video_result> project::import_video(const std::filesystem::path& filepath, video_id_t id, bool create_group)
	{
		static std::mutex videos_mutex;
		static std::mutex groups_mutex;

		auto task = [filepath, id, create_group, this]() mutable
		{
			if (id == 0)
			{
				id = utils::hash::fnv_hash(filepath); //utils::uuid::get()
			}

			{
				std::scoped_lock lock(videos_mutex);
				if (!videos.insert(id, filepath))
				{
					return project_import_video_result{ false, 0, filepath };
				}
			}

			if (create_group)
			{
				video_group::video_info group_info{};
				group_info.id = id;

				auto gid = (ctx_.current_video_group_id == invalid_video_group_id) ? utils::uuid::get() : ctx_.current_video_group_id;
				
				std::scoped_lock lock(groups_mutex);
				auto& group = video_groups[gid];
				group.display_name = filepath.stem().u8string();
				group.insert(group_info);
			}

			{
				std::scoped_lock lock(videos_mutex);
				videos.get(id)->update_data();
			}

			ctx_.is_project_dirty = create_group;
			return project_import_video_result{ true, id, filepath };
		};

		
		return std::async(std::launch::async, task);
	}
	
	void project::save() const
	{
		//TODO: Add error checking

		nlohmann::ordered_json json;
		auto& project = json["project"];
		project["version"] = version;
		project["name"] = name;

		auto& json_tags = json["tags"];
		json_tags = tags;

		auto& json_segments = json["segments"];
		json_segments = nlohmann::json::array();
		for (auto& [group_id, group_segments] : segments)
		{
			nlohmann::ordered_json json_group_segments_data;
			json_group_segments_data["group-id"] = group_id;
			auto& json_group_segments = json_group_segments_data["group-segments"];
			json_group_segments = nlohmann::json::array();
			for (auto& [tag_name, tag_segments] : group_segments)
			{
				nlohmann::ordered_json json_tag_segments_data;
				json_tag_segments_data["tag"] = tag_name;
				auto& json_tag_segments = json_tag_segments_data["tag-segments"];
				json_tag_segments = nlohmann::json::array();
				for (auto& segment : tag_segments)
				{
					nlohmann::ordered_json segment_json;
					switch (segment.type())
					{
					case tag_segment_type::timestamp:
					{
						segment_json["timestamp"] = utils::time::time_to_string(segment.start.seconds_total.count());
					}
					break;
					case tag_segment_type::segment:
					{
						segment_json["start"] = utils::time::time_to_string(segment.start.seconds_total.count());
						segment_json["end"] = utils::time::time_to_string(segment.end.seconds_total.count());
					}
					break;
					}
					json_tag_segments.push_back(segment_json);
				}
				json_group_segments.push_back(json_tag_segments_data);
			}
			json_segments.push_back(json_group_segments_data);
		}

		if (!videos.empty())
		{
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
		}

		if (!video_groups.empty())
		{
			auto& json_groups = json["groups"];
			json_groups = nlohmann::json::array();
			for (auto& [id, group] : video_groups)
			{
				nlohmann::ordered_json json_group;
				json_group["id"] = id;
				json_group["name"] = group.display_name;
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
		}

		if (!keybinds.empty())
		{
			json["keybinds"] = keybinds;
		}

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

			if (json.contains("tags") and json.at("tags").is_array())
			{
				result.tags = json["tags"];
			}

			if (json.contains("segments") and json.at("segments").is_array())
			{
				for (auto& json_segments : json["segments"])
				{
					if (!json_segments.contains("group-id"))
					{
						debug::error("Missing group id");
						continue;
					}
					if (!json_segments.contains("group-segments"))
					{
						debug::error("Missing group segments");
						continue;
					}

					video_group_id_t group_id = json_segments["group-id"];
					auto& group_segments = result.segments[group_id];
					for (auto& json_group_segments : json_segments["group-segments"])
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
						auto& tag_segments = group_segments[tag_name];
						for (auto& json_tag_segments : json_group_segments["tag-segments"])
						{
							if (json_tag_segments.contains("timestamp"))
							{
								auto ts = utils::time::parse_time_to_sec(json_tag_segments["timestamp"]);
								tag_segments.insert(vt::timestamp{ ts });
							}
							else if (json_tag_segments.contains("start") and json_tag_segments.contains("end"))
							{
								auto start = utils::time::parse_time_to_sec(json_tag_segments["start"]);
								auto end = utils::time::parse_time_to_sec(json_tag_segments["end"]);
								tag_segments.insert(vt::timestamp{ start }, vt::timestamp{ end });
							}
						}
					}
				}
			}

			if (json.contains("videos") and json.at("videos").is_array())
			{
				const auto& videos = json["videos"];
				for (const auto& video : videos)
				{
					if (!video.contains("id"))
					{
						debug::error("Project's video doesn't contain id, skipping...");
						continue;
					}

					if (!video.contains("path"))
					{
						debug::error("Project's video doesn't contain a filepath, skipping...");
						continue;
					}

					video_id_t id = video["id"];
					std::filesystem::path path = video["path"];

					//TODO: Can't be async because result is later moved
					result.import_video(path, id, false);
					auto pool_data = result.videos.get(id);
					if (pool_data != nullptr and ctx_.app_settings.load_thumbnails)
					{
						pool_data->update_thumbnail(ctx_.renderer);
					}
				}
			}

			if (json.contains("groups") and json.at("groups").is_array())
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
					
					if (group.contains("name"))
					{
						vgroup.display_name = group["name"];
					}
					else
					{
						vgroup.display_name = std::to_string(id);
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
