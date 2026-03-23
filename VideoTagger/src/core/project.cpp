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
#include <video/downloadable_video_resource.hpp>
#include <video/video_resource.hpp>

#include <events/player/video_group_change_request_event.hpp>

#include <events/video_resource/video_load_thumbnail_request_event.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

	bool project::import_video(std::unique_ptr<video_resource>&& vid_resource, std::optional<video_group_id_t> group_id, bool check_hash, bool set_project_dirty)
	{
		if (vid_resource == nullptr)
		{
			debug::error("Failed to import video");
			return false;
		}

		const auto& metadata = vid_resource->metadata();

		if (check_hash and vid_resource->metadata().sha256.has_value())
		{
			for (auto& [id, video] : videos)
			{
				const auto& hash = *video->metadata().sha256;
				if (hash == vid_resource->metadata().sha256)
				{
					debug::warn("Video with hash: {} is already imported", utils::hash::bytes_to_hex(hash, utils::hash::string_case::lower));
					return false;
				}
			}
		}

		video_group::video_info group_info{};
		group_info.id = vid_resource->id();
		
		if (!videos.insert(std::move(vid_resource)))
		{
			return false;
		}

		if (group_id.has_value())
		{
			auto group_it = video_groups.find(*group_id);
			if (group_it == video_groups.end())
			{
				auto& group = video_groups[*group_id];
				group.display_name = metadata.title.has_value() ? *metadata.title : fmt::format("Untitled Group {}", group_info.id);
				group.insert(group_info);
			}
			else
			{
				auto& group = video_groups.at(*group_id);
				group.insert(group_info);
			}
		}

		if (set_project_dirty)
		{
			ctx_.is_project_dirty = true;
		}

		return true;
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
			if (group_it == video_groups.end())
			{
				//TODO: Should this do something?
				continue;
			}

			auto& group = group_it->second;

			nlohmann::ordered_json json_group;

			json_group["name"] = group.display_name;
			json_group["id"] = std::to_string(group_id);
			
			{
				auto& json_group_videos = json_group["videos"];
				json_group_videos = nlohmann::ordered_json::array();
				for (auto& group_video_info : group)
				{
					auto& vid_resource = videos.get(group_video_info.id);
					const auto& metadata = vid_resource.metadata();

					auto json_video = nlohmann::ordered_json::object();

					json_video["title"] = metadata.title.has_value() ? *metadata.title : "UNKNOWN";
					json_video["id"] = std::to_string(group_video_info.id);
					if (metadata.sha256.has_value())
					{
						json_video["sha256"] = utils::hash::bytes_to_hex(*metadata.sha256, utils::hash::string_case::lower);
					}
					json_video["offset"] = timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(group_video_info.offset) };
					json_group_videos.push_back(json_video);
				}
			}

			{
				auto& json_tags = json_group["tags"];
				json_tags = nlohmann::ordered_json::array();
				for (auto& [tag_name, _] : group.segments())
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
					for (auto& [tag_name, tag_segments] : group.segments())
					{
						auto json_tag_data = nlohmann::ordered_json::object();
						json_tag_data["tag"] = tag_name;

						auto& json_tag_segments = json_tag_data["segments"];
						json_tag_segments = nlohmann::ordered_json::array();
						for (auto [segment_id, segment] : tag_segments) //copy is intended
						{
							segment.start -= timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(group_video_info.offset) };
							segment.end -= timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(group_video_info.offset) };

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

		if (!videos.empty())
		{
			auto& json_videos = json["videos"];

			for (const auto& [id, vid_resource] : videos)
			{
				if (!json_videos.contains(vid_resource->importer_id()))
				{
					json_videos[vid_resource->importer_id()] = nlohmann::json::array();
				}

				json_videos.at(vid_resource->importer_id()).push_back(vid_resource->save());
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

				json_group["segments"] = group.segments();

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
		for (auto& tag_name : displayed_tags)
		{
			json_displayed_tags.push_back(tag_name);
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

	void project::remove_video(video_id_t id)
	{
		debug::log("Removing video with id: {}", id);

		std::vector<video_group_id_t> groups_to_remove;

		for (auto& [group_id, group] : video_groups)
		{
			if (!group.contains(id))
			{
				continue;
			}

			group.erase(id);

			if (group.empty())
			{
				groups_to_remove.push_back(group_id);
			}
		}

		for (auto& group_id : groups_to_remove)
		{
			remove_video_group(group_id);
		}

		ctx_.displayed_videos.erase(id);

		//TODO: Should somehow stop or wait for tasks
		
		//{
		//	auto it = std::find_if(load_thumbnail_tasks.begin(), load_thumbnail_tasks.end(), [id](const auto& task) { return task.video_id == id; });
		//	if (it != load_thumbnail_tasks.end())
		//	{
		//		load_thumbnail_tasks.erase(it);
		//	}
		//}
		//
		//{
		//	auto it = std::find_if(video_download_tasks.begin(), video_download_tasks.end(), [id](const auto& task) { return task.video_id == id; });
		//	if (it != video_download_tasks.end())
		//	{
		//		it->task.cancel();
		//		it->task.result.get();
		//		video_download_tasks.erase(it);
		//	}
		//}

		if (videos.erase(id))
		{
			ctx_.is_project_dirty = true;
		}
	}

	void project::remove_video_group(video_group_id_t id)
	{
		if (ctx_.session.current_video_group_id() == id)
		{
			ctx_.dispatch_event<video_group_change_request_event>("project", ctx_.get_window<widgets::video_player>(), invalid_video_group_id);
		}

		auto playlist_it = std::find(video_group_playlist.begin(), video_group_playlist.end(), id);
		if (playlist_it != video_group_playlist.end())
		{
			video_group_playlist.erase(playlist_it);
		}

		video_groups.erase(id);

		ctx_.is_project_dirty = true;
	}

	bool project::add_displayed_tag(const std::string& tag_name)
	{
		auto it = std::lower_bound(displayed_tags.begin(), displayed_tags.end(), tag_name);
		if (it != displayed_tags.end() and *it == tag_name)
		{
			return false;
		}

		displayed_tags.insert(it, tag_name);
		return true;
	}

	bool project::remove_displayed_tag(const std::string& tag_name)
	{
		auto it = std::lower_bound(displayed_tags.begin(), displayed_tags.end(), tag_name);
		if (it == displayed_tags.end() or *it != tag_name)
		{
			return false;
		}

		displayed_tags.erase(it);
		return true;
	}

	std::vector<std::string>::iterator project::find_displayed_tag(const std::string& tag_name)
	{
		auto it = std::lower_bound(displayed_tags.begin(), displayed_tags.end(), tag_name);
		if (it == displayed_tags.end() or *it != tag_name)
		{
			return displayed_tags.end();
		}
		
		return it;
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

			if (json.contains("videos") and json.at("videos").is_object())
			{
				const auto& videos_json = json["videos"];
				for (auto& [importer_id, importer] : ctx_.video_importers)
				{
					if (!videos_json.contains(importer_id))
					{
						continue;
					}

					for (auto& video_json : videos_json.at(importer_id))
					{
						auto vid_resource = importer->import_video(video_json);
						if (vid_resource == nullptr)
						{
							continue;
						}

						video_id_t video_id = vid_resource->id();
						if (result.import_video(std::move(vid_resource), std::nullopt, false, false))
						{
						}
					}
				}
			}

			if (json.contains("groups") and json.at("groups").is_array())
			{
				const auto& json_groups = json["groups"];
				for (auto& json_group : json_groups)
				{
					video_group vgroup;
					if (!json_group.contains("id"))
					{
						debug::error("Project's video group doesn't contain id, skipping...");
						continue;
					}


					video_group_id_t id = json_group["id"];
					if (!json_group.contains("videos") or !json_group.at("videos").is_array())
					{
						debug::error("Project's video group's videos format was invalid, skipping...");
						continue;
					}
					
					if (json_group.contains("name"))
					{
						vgroup.display_name = json_group["name"];
					}
					else
					{
						vgroup.display_name = std::to_string(id);
					}

					const auto& group_videos = json_group.at("videos");
					for (const auto& group_video : group_videos)
					{
						if (!group_video.contains("id") or !group_video.contains("offset"))
						{
							debug::error("Video's format was invalid, skipping...");
							continue;
						}

						video_group::video_info vinfo;
						vinfo.id = group_video["id"];
						vinfo.offset = (decltype(vinfo.offset))utils::time::parse_time_to_ms(group_video["offset"]);
						vgroup.insert(vinfo);
					}

					if (json_group.contains("segments") and json_group.at("segments").is_array())
					{
						from_json(json_group["segments"], vgroup.segments(), result.tags);
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
				if (json_video_timeline.contains("displayed-tags") and json_video_timeline.at("displayed-tags").is_array())
				{
					for (auto& tag_name : json_video_timeline.at("displayed-tags"))
					{
						result.add_displayed_tag(tag_name);
					}
				}
			}
		}

		return result;
	}
}
