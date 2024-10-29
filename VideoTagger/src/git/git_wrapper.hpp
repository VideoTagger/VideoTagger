#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

struct subprocess_s;

namespace vt::git
{
	enum class path_status
	{
		ok,
		not_found,
		wrong_file_type
	};

	enum class command_result_status
	{
		start_failed,
		running,
		finished
	};

	struct command_result_failed_construct_t { explicit command_result_failed_construct_t() = default; };
	inline constexpr command_result_failed_construct_t command_result_failed_construct;

	class command_result
	{
	public:
		command_result(subprocess_s&& process_handle);
		command_result(command_result_failed_construct_t);
		~command_result();

		command_result_status status();

		std::optional<int> return_value();

		bool terminate();

		subprocess_s& process_handle();
		const subprocess_s& process_handle() const;

	private:
		std::unique_ptr<subprocess_s> process_handle_;
		std::optional<int> return_value_;
	};

	class git_wrapper
	{
	public:
		git_wrapper(std::filesystem::path git_path, std::filesystem::path repository_path);

		path_status check_git_path() const;
		path_status check_repository_path() const;

		const std::filesystem::path& git_path() const;
		const std::filesystem::path& repository_path() const;

		command_result execute_command(const std::string& command, const std::vector<std::string>& arguments) const;
	private:
		std::filesystem::path git_path_;
		std::filesystem::path repository_path_;
	};

	extern std::filesystem::path get_global_git_path();
}
