#pragma once
#include <string>
#include <filesystem>
#include <map>
#include <optional>

#include <core/input.hpp>
#include <tags/tag_storage.hpp>

namespace vt
{
	struct project
	{
		static constexpr const char* extension = "vtproj";

		uint16_t version = 1;
		std::string name = "New Project";
		std::filesystem::path path = std::filesystem::current_path();

		tag_storage tags;
		std::map<std::string, keybind> keybinds;

		//TODO: Store tags displayed on the timeline

		//TODO: Add keybinds

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save() const;
		void save_as(const std::filesystem::path& filepath);

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
