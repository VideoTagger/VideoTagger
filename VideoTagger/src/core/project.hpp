#pragma once
#include <string>
#include <filesystem>
#include <map>

#include "tags/tag.hpp"

namespace vt
{
	struct project
	{
		std::string name;
		std::filesystem::path path;
		std::filesystem::path working_dir;

		//Custom tags provided by the user
		std::map<std::string, tag> tags;

		//TODO: Add keybinds

		bool is_valid() const;
		void save() const;

		bool operator==(const project& other) const;

		static project load_from_file(const std::filesystem::path& filepath);
	};
}
