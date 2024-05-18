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

				auto gid = (ctx_.current_video_group_id() == invalid_video_group_id) ? utils::uuid::get() : ctx_.current_video_group_id();
				
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

	bool project::export_segments(const std::filesystem::path& filepath, std::vector<video_group_id_t> group_ids) const
	{
		if (group_ids.empty())
		{
			return false;
		}

		nlohmann::ordered_json json;
		json["version"] = version;
		auto& json_groups = json["groups"];
		json_groups = nlohmann::ordered_json::array();

		for (const auto& group_id : group_ids)
		{
			auto group_it = video_groups.find(group_id);
			auto segments_it = segments.find(group_id);
			if (group_it == video_groups.end() or segments_it == segments.end())
			{
				//TODO: Should this do something?
				continue;
			}

			auto& group = group_it->second;
			auto& group_segments = segments_it->second;

			nlohmann::ordered_json json_group;

			json_group["name"] = group.display_name;
			json_group["id"] = std::to_string(group_id);
			
			{
				auto& json_group_videos = json_group["videos"];
				json_group_videos = nlohmann::ordered_json::array();
				for (auto& group_video_info : group)
				{
					auto* video_metadata = videos.get(group_video_info.id);
					if (video_metadata == nullptr)
					{
						//TODO: Should this do something?
						continue;
					}

					auto json_video = nlohmann::ordered_json::object();
					json_video["name"] = video_metadata->path.filename().u8string(); //TODO: Should this be the filename
					json_video["id"] = std::to_string(group_video_info.id);
					json_group_videos.push_back(json_video);
				}
			}

			{
				auto& json_tags = json_group["tags"];
				json_tags = nlohmann::ordered_json::array();
				for (auto& [tag_name, _] : group_segments)
				{
					json_tags.push_back(tag_name);
				}
			}
			
			{
				auto& json_group_segments = json_group["segments"];

				for (auto& group_video_info : group)
				{
					auto& json_video_segments = json_group_segments[std::to_string(group_video_info.id)];
					json_video_segments = nlohmann::ordered_json::array();
					for (auto& [tag_name, tag_segments] : group_segments)
					{
						auto json_tag_data = nlohmann::ordered_json::object();
						json_tag_data["tag"] = tag_name;

						auto& json_tag_segments = json_tag_data["segments"];
						json_tag_segments = nlohmann::ordered_json::array();
						for (tag_segment segment : tag_segments) //copy is intended
						{
							segment.start -= timestamp{ std::chrono::duration_cast<std::chrono::seconds>(group_video_info.offset) };
							segment.end -= timestamp{ std::chrono::duration_cast<std::chrono::seconds>(group_video_info.offset) };

							bool clipped_start = false;
							bool clipped_end = false;

							if (segment.start < timestamp::zero())
							{
								segment.start = timestamp::zero();
								clipped_start = true;
							}

							if (segment.end < timestamp::zero())
							{
								segment.end = timestamp::zero();
								clipped_end = true;
							}

							if (clipped_start and clipped_end)
							{
								continue;
							}

							json_tag_segments.push_back(segment);
						}

						json_video_segments.push_back(json_tag_data);
					}
				}
			}

			json_groups.push_back(json_group);
		}

		return utils::json::write_to_file(json, filepath);
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

		if (!segments.empty())
		{
			for (auto& [group_id, group_segments] : segments)
			{
				nlohmann::ordered_json json_group_segments_data;
				json_group_segments_data["group-id"] = group_id;
				auto& json_group_segments = json_group_segments_data["group-segments"];
				json_group_segments = group_segments;
				json_segments.push_back(json_group_segments_data);
			}
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

		if (!video_group_playlist.empty())
		{
			json["group-queue"] = video_group_playlist;
		}

		if (!keybinds.empty())
		{
			json["keybinds"] = keybinds;
		}

		auto& json_video_timeline = json["video-timeline"];
		auto& json_displayed_tags = json_video_timeline["displayed-tags"];
		json_displayed_tags = nlohmann::ordered_json::array();
		for (auto& [group_id, displayed_tags] : ctx_.video_timeline.displayed_tags_per_group())
		{
			auto json_group_tags = nlohmann::ordered_json::object();
			json_group_tags["group-id"] = group_id;
			json_group_tags["tags"] = displayed_tags;
			json_displayed_tags.push_back(json_group_tags);
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
					group_segments = json_segments["group-segments"];
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

			if (json.contains("group-queue") and json.at("group-queue").is_array())
			{
				result.video_group_playlist = json.at("group-queue");
			}

			if (json.contains("keybinds"))
			{
				result.keybinds = json["keybinds"];
			}

			if (json.contains("video-timeline"))
			{
				auto& json_video_timeline = json.at("video-timeline");
				if (json_video_timeline.contains("displayed-tags"))
				{
					for (auto& json_group_tags : json_video_timeline.at("displayed-tags"))
					{
						auto& timeline_displayed_tags = ctx_.video_timeline.displayed_tags_per_group();
						if (json_group_tags.contains("group-id") and json_group_tags.contains("tags"))
						{
							timeline_displayed_tags[json_group_tags.at("group-id")] = json_group_tags.at("tags");
						}
					}
				}
			}
		}

		return result;
	}
}
