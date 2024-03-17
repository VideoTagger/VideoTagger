#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <optional>

#include <tags/tag_storage.hpp>
#include <video/video_pool.hpp>


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
		video_pool videos;
		std::unordered_map<group_id_t, video_group> video_groups;

		//TODO: Store tags displayed on the timeline

		//TODO: Add keybinds

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save() const;
		void save_as(const std::filesystem::path& filepath);

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
		static project shallow_copy(const project& other);
	};
}
