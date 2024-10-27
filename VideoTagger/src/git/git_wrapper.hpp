#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace vt
{
	enum class git_wrapper_path_status
	{
		ok,
		not_found,
		wrong_file_type,
		insufficient_permissions
	};

	class git_wrapper
	{
	public:
		git_wrapper(std::filesystem::path git_path, std::filesystem::path repository_path);

		git_wrapper_path_status check_git_path() const;
		git_wrapper_path_status check_repository_path() const;

		const std::filesystem::path& git_path() const;
		const std::filesystem::path& repository_path() const;

		int execute_command(const std::string& command, const std::vector<std::string>& arguments = {}) const;

	private:
		std::filesystem::path git_path_;
		std::filesystem::path repository_path_;
	};
}
