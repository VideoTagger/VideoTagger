#pragma once
#include <vector>
#include <filesystem>

namespace vt::utils
{
	enum class dialog_option
	{
		error,
		ok,
		cancel
	};

	struct dialog_result
	{
		dialog_option option;
		std::filesystem::path path;

		constexpr operator bool() const
		{
			return option == dialog_option::ok;
		}
	};

	struct dialog_results
	{
		dialog_option option;
		std::vector<std::filesystem::path> path;

		constexpr operator bool() const
		{
			return option == dialog_option::ok;
		}
	};

	struct dialog_filter
	{
		std::string description;
		std::string extensions;
	};

	using dialog_filters = std::vector<dialog_filter>;

	struct filesystem
	{
		static dialog_result get_file(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {});
		static dialog_result get_folder(const std::filesystem::path& start_dir = {});
		static dialog_result save_file(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {}, const std::string& default_filename = {});
		static dialog_results get_paths(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {});
	};
}