#pragma once
#include <map>
#include <filesystem>
#include <set>
#include <vector>

namespace vt::utils
{
	struct file_node
	{
		using folder_container = std::map<std::filesystem::path, file_node>;

		folder_container folders;
		std::set<std::filesystem::path> children;

		void clear()
		{
			children.clear();
			for (auto& [path, folder] : folders)
			{
				folder.clear();
			}
		}

		void insert(const std::filesystem::path& path)
		{
			children.insert(path);
		}

		bool empty() const
		{
			bool result = children.empty();
			for (const auto& [_, folder] : folders)
			{
				result &= folder.empty();
			}
			return result;
		}

		std::vector<std::filesystem::path> all_children() const
		{
			std::vector<std::filesystem::path> result;
			for (const auto& [_, folder] : folders)
			{
				auto folder_children = folder.all_children();
				result.insert(result.end(), folder_children.begin(), folder_children.end());
			}
			result.insert(result.end(), children.begin(), children.end());
			return result;
		}

		folder_container::iterator begin()
		{
			return folders.begin();
		}

		folder_container::const_iterator begin() const
		{
			return folders.begin();
		}

		folder_container::iterator end()
		{
			return folders.end();
		}

		folder_container::const_iterator end() const
		{
			return folders.end();
		}

		file_node& at(const std::filesystem::path& path)
		{
			return folders[path];
		}

		const file_node& at(const std::filesystem::path& path) const
		{
			return folders.at(path);
		}

		file_node& operator[](const std::filesystem::path& path)
		{
			return at(path);
		}

		const file_node& operator[](const std::filesystem::path& path) const
		{
			return at(path);
		}
	};
}
