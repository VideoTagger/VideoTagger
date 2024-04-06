#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <optional>

#include "keybind_storage.hpp"
#include "input.hpp"
#include <tags/tag_storage.hpp>
#include <tags/tag_timeline.hpp>
#include <video/video_pool.hpp>
#include <core/input.hpp>

namespace vt
{
	using video_group_id_t = uint64_t;

	struct project_info
	{
		static constexpr uint16_t current_version = 1;
		static constexpr const char* extension = "vtproj";

		uint16_t version = current_version;
		std::string name = "New Project";
		std::filesystem::path path = std::filesystem::current_path();

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save();

		bool operator==(const project_info& other) const;

		static project_info load_from_file(const std::filesystem::path& filepath);
	};

	struct project : public project_info
	{
		tag_storage tags;
		//TODO: associate segments with video groups
		segment_storage segments;
		video_pool videos;
		std::unordered_map<video_group_id_t, video_group> video_groups;
		keybind_storage keybinds;

		//TODO: save tags displayed in the timeline in the project file

		bool import_video(const std::filesystem::path& filepath, video_id_t id = 0, bool create_group = true);

		void save() const;
		void save_as(const std::filesystem::path& filepath);

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
