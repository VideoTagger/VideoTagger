#pragma once
#include <string>
#include <filesystem>
#include <map>

#include "tags/tag_storage.hpp"

namespace vt
{
	struct project
	{
		static constexpr const char* extension = "json";

		std::string name = "New Project";
		std::filesystem::path path = std::filesystem::current_path();
		std::filesystem::path working_dir = ".";

		tag_storage tags;

		//TODO: Add keybinds

		bool is_valid() const;
		void save() const;

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
