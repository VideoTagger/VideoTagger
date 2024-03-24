#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <optional>

#include <tags/tag_storage.hpp>
#include <tags/tag_timeline.hpp>
#include <video/video_pool.hpp>
#include <core/input.hpp>

namespace vt
{
	using group_id_t = uint64_t;

	struct project
	{
		static constexpr const char* extension = "vtproj";

		uint16_t version = 1;
		std::string name = "New Project";
		std::filesystem::path path = std::filesystem::current_path();

		tag_storage tags;
		//TODO: associate segments with video groups
		segment_storage segments;
		video_pool videos;
		std::unordered_map<group_id_t, video_group> video_groups;
		std::map<std::string, keybind> keybinds;

		//TODO: save tags displayed in the timeline in the project file

		bool import_video(const std::filesystem::path& file_path, SDL_Renderer* renderer);

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save() const;
		void save_as(const std::filesystem::path& filepath);

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
		static project shallow_copy(const project& other);
	};
}
