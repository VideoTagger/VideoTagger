#pragma once
#include <string>
#include <filesystem>
#include <map>
#include <optional>

#include "tags/tag.hpp"

namespace vt
{
	struct project
	{
		static constexpr const char* extension = "json";

		std::string name = "New Project";
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path working_dir = ".";

		//Custom tags provided by the user
		std::map<std::string, tag> tags;

		//TODO: Store tags displayed on the timeline

		//TODO: Add keybinds

		bool is_valid() const;
		std::optional<std::tm> modification_time() const;
		void save() const;

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
