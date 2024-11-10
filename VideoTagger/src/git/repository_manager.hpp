#pragma once
#include <git/git_wrapper.hpp>
#include <filesystem>

namespace vt::git
{
	class repository_manager
	{
	public:
		repository_manager() = default;
		repository_manager(const std::filesystem::path& repository_path);

	private:
		git::git_wrapper git_wrapper_;
	};
}
