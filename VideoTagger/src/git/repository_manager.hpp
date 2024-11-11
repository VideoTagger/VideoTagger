#pragma once
#include <git/git_wrapper.hpp>
#include <filesystem>
#include <future>
#include <variant>

namespace vt::git
{
	struct error_type
	{
		bool has_error;
		std::string message;
	};

	class repository_manager
	{
	public:
		repository_manager() = default;
		repository_manager(const std::filesystem::path& repository_path);

	private:
		git::git_wrapper git_wrapper_;
	};

	extern std::future<std::variant<repository_manager, error_type>> initialize_repository(const std::filesystem::path& directory);
}
