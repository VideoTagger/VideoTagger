#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include <tasks/cancellation_token.hpp>
#include <httplib.h>
#include <functional>

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
		std::vector<std::filesystem::path> paths;

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

	//TODO: Why isn't this a namespace???
	struct filesystem
	{
		static dialog_result get_file(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {});
		static dialog_result get_folder(const std::filesystem::path& start_dir = {});
		static dialog_result save_file(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {}, const std::string& default_filename = {});
		static dialog_results get_files(const std::filesystem::path& start_dir = {}, const dialog_filters& filters = {});

		static std::string normalize(const std::filesystem::path& filepath);

		static void open_in_explorer(const std::filesystem::path& path);
		static void open_file_in_explorer(const std::filesystem::path& path);

		static std::string concat_extensions(const std::vector<std::string>& extensions);

		static std::filesystem::path get_storage_path(const std::string& organization, const std::string& app_name);

		static bool is_subdirectory(const std::filesystem::path& parent, const std::filesystem::path& child);

		static bool download_file(const std::string& url, const std::filesystem::path& destination, std::optional<httplib::Headers> headers,
			std::optional<cancellation_token> cancel_token, std::function<void(uint64_t current_size, uint64_t total_size, std::optional<cancellation_token> cancel_token)> callback);
	};
}
