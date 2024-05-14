#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <future>
#include <memory>

#include "keybind_storage.hpp"
#include <tags/tag_storage.hpp>
#include <tags/tag_timeline.hpp>
#include <video/video_pool.hpp>
#include <video/video_group_playlist.hpp>
#include <core/input.hpp>

namespace vt
{
	struct project_info
	{
		static constexpr uint16_t current_version = 1;
		static constexpr const char* extension = "vtproj";

		uint16_t version = current_version;
		std::string name = "New Project";
		std::filesystem::path path = (std::filesystem::current_path() / name).replace_extension(extension);

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save();

		bool operator==(const project_info& other) const;

		static project_info load_from_file(const std::filesystem::path& filepath);
	};

	struct project_import_video_result
	{
		bool success{};
		video_id_t video_id{};
		std::filesystem::path video_path;
	};

	struct project : public project_info
	{
		using segment_storage_map = std::unordered_map<video_group_id_t, segment_storage>;
		using video_group_map = std::unordered_map<video_group_id_t, video_group>;

		video_group_playlist video_group_playlist;
		segment_storage_map segments;
		video_group_map video_groups;
		std::vector<std::future<project_import_video_result>> video_import_tasks;
		video_pool videos;
		tag_storage tags;
		keybind_storage keybinds;


		project() = default;
		project(const project&) = delete;
		project(project&&) = default;

		project& operator=(const project&) = delete;
		project& operator=(project&&) = default;

		std::future<project_import_video_result> import_video(const std::filesystem::path& filepath, video_id_t id = 0, bool create_group = true);
		bool export_segments(const std::filesystem::path& filepath, std::vector<video_group_id_t> group_ids) const;

		//TODO: save tags displayed on the timeline in the project file
		void save() const;
		void save_as(const std::filesystem::path& filepath);

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
