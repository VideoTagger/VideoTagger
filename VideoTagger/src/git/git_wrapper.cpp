#include "pch.hpp"
#include "git_wrapper.hpp"

#include "subprocess.hpp"

namespace vt
{
	git_wrapper::git_wrapper(std::filesystem::path git_path, std::filesystem::path repository_path)
		: git_path_{ git_path }, repository_path_{ repository_path }
	{
	}

	git_wrapper_path_status git_wrapper::check_git_path() const
	{
		std::filesystem::file_status file_status = std::filesystem::status(git_path_);

		if (file_status.type() == std::filesystem::file_type::not_found)
		{
			return git_wrapper_path_status::not_found;
		}

		if (file_status.type() != std::filesystem::file_type::regular)
		{
			return git_wrapper_path_status::wrong_file_type;
		}

		std::filesystem::perms file_permissions = file_status.permissions();
		//TODO: check permission
	}
}


