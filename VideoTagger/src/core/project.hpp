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
#include <video/downloadable_video_resource.hpp>
#include <video/video_group_playlist.hpp>
#include <video/video_importer.hpp>
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

	struct prepare_video_import_task
	{
		std::string importer_id;
		std::vector<std::any> import_data;
		std::function<bool(std::vector<std::any>&)> task;

		bool operator()();
	};

	struct video_import_task
	{
		std::optional<video_group_id_t> group_id;
		std::function<std::unique_ptr<video_resource>()> task;

		std::unique_ptr<video_resource> operator()();
	};

	struct generate_thumbnail_task
	{
		video_id_t video_id;
		std::function<bool()> task;

		bool operator()();
	};

	struct video_download_task
	{
		video_id_t video_id;
		video_download_result result;
	};

	struct project : public project_info
	{
		using video_group_map = std::unordered_map<video_group_id_t, video_group>;

		video_group_playlist video_group_playlist;
		video_group_map video_groups;
		video_pool videos;
		tag_storage tags;
		keybind_storage keybinds;

		//TODO: maybe use async
		std::vector<prepare_video_import_task> prepare_video_import_tasks;
		std::vector<video_import_task> video_import_tasks;
		std::vector<generate_thumbnail_task> generate_thumbnail_tasks;
		std::vector<video_download_task> make_available_tasks;

		project() = default;
		project(const project&) = delete;
		project(project&&) = default;

		project& operator=(const project&) = delete;
		project& operator=(project&&) = default;

		template<typename VideoImporter>
		void prepare_video_import();
		void prepare_video_import(const std::string& importer_id);

		template<typename VideoImporter>
		void schedule_video_import(typename VideoImporter::import_data import_data, std::optional<video_group_id_t> group_id);
		void schedule_video_import(const std::string& importer_id, std::any import_data, std::optional<video_group_id_t> group_id);

		void schedule_video_download(video_id_t video_id);

		void schedule_generate_thumbnail(video_id_t video_id);

		bool import_video(std::unique_ptr<video_resource>&& vid_resource, std::optional<video_group_id_t> group_id, bool check_hash = true, bool set_project_dirty = true);

		bool export_segments(const std::filesystem::path& filepath, std::vector<video_group_id_t> group_ids) const;

		//TODO: save tags displayed on the timeline in the project file
		void save() const;
		void save_as(const std::filesystem::path& filepath);

		void remove_video(video_id_t id);
		void remove_video_group(video_group_id_t id);

		tag_rename_result rename_tag(const std::string& old_name, const std::string& new_name);
		void delete_tag(const std::string& tag_name);

		static project load_from_file(const std::filesystem::path& filepath);
	};

	template<typename VideoImporter>
	inline void project::prepare_video_import()
	{
		return prepare_video_import(VideoImporter::static_importer_id);
	}

	template<typename VideoImporter>
	inline void project::schedule_video_import(typename VideoImporter::import_data import_data, std::optional<video_group_id_t> group_id)
	{
		return schedule_video_import(VideoImporter::static_importer_id, std::move(import_data), group_id);
	}
}
