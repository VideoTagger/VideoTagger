#include "pch.hpp"
#include <cstdlib>
#include "git_wrapper.hpp"
#include <core/debug.hpp>

#include "subprocess.hpp"

namespace vt::git
{
	command_result::command_result(subprocess_s&& process_handle)
		: process_handle_{ std::make_unique<subprocess_s>() }
	{
		*process_handle_ = process_handle;
		std::memset(&process_handle, 0, sizeof(subprocess_s));
	}

	command_result::command_result(command_result_failed_construct_t)
	{
	}

	command_result::~command_result()
	{
		if (process_handle_ != nullptr)
		{
			subprocess_destroy(process_handle_.get());
		}
	}

	command_result_status command_result::status()
	{
		if (process_handle_ == nullptr)
		{
			return command_result_status::start_failed;
		}

		if (subprocess_alive(process_handle_.get()))
		{
			return command_result_status::running;
		}

		return command_result_status::finished;
	}

	static std::string read_FILE(FILE* file)
	{
		std::array<char, 4096> read_buffer{};

		std::string result;

		size_t bytes_read{};
		while (bytes_read = fread(read_buffer.data(), 1, read_buffer.size(), file))
		{
			result.insert(result.end(), read_buffer.begin(), read_buffer.begin() + bytes_read);
		}

		return result;
	}

	std::string command_result::read_stdout()
	{
		if (status() != command_result_status::finished)
		{
			return std::string();
		}

		return read_FILE(subprocess_stdout(process_handle_.get()));
	}

	std::string command_result::read_stderr()
	{
		if (status() != command_result_status::finished)
		{
			return std::string();
		}

		return read_FILE(subprocess_stderr(process_handle_.get()));
	}

	std::optional<int> command_result::return_value()
	{
		if (!return_value_.has_value())
		{
			int ret = 0;
			if (subprocess_join(process_handle_.get(), &ret) == 0)
			{
				return_value_ = ret;
			}
		}

		return return_value_;
	}

	bool command_result::terminate()
	{
		if (status() != command_result_status::running)
		{
			return true;
		}

		return subprocess_terminate(process_handle_.get());
	}

	subprocess_s& command_result::process_handle()
	{
		return *process_handle_;
	}

	const subprocess_s& command_result::process_handle() const
	{
		return *process_handle_;
	}

	git_wrapper::git_wrapper(std::filesystem::path git_path, std::filesystem::path repository_path)
		: git_path_{ git_path }, repository_path_{ repository_path }
	{
	}

	path_status git_wrapper::check_git_path() const
	{
		std::filesystem::file_status file_status = std::filesystem::status(git_path_);

		if (file_status.type() == std::filesystem::file_type::not_found)
		{
			return path_status::not_found;
		}

		if (file_status.type() != std::filesystem::file_type::regular)
		{
			return path_status::wrong_file_type;
		}

		return path_status::ok;
	}

	path_status git_wrapper::check_repository_path() const
	{
		std::filesystem::file_status file_status = std::filesystem::status(repository_path_);

		if (file_status.type() == std::filesystem::file_type::not_found)
		{
			return path_status::not_found;
		}

		if (file_status.type() != std::filesystem::file_type::directory)
		{
			return path_status::wrong_file_type;
		}

		return path_status::ok;
	}

	const std::filesystem::path& git_wrapper::git_path() const
	{
		return git_path_;
	}

	const std::filesystem::path& git_wrapper::repository_path() const
	{
		return repository_path_;
	}

	command_result git_wrapper::execute_command(const std::string& command, const std::vector<std::string>& arguments) const
	{
		std::vector<const char*> process_args;
		process_args.reserve((4 + arguments.size() + 1));
		
		std::string git_path_string = git_path_.u8string();
		std::string rep_path_string = repository_path_.u8string();

		process_args.push_back(git_path_string.c_str());
		process_args.push_back("-C");
		process_args.push_back(rep_path_string.c_str());

		process_args.push_back(command.c_str());

		for (const auto& arg : arguments)
		{
			process_args.push_back(arg.c_str());
		}

		process_args.push_back(nullptr);

		subprocess_s process_handle{};
		if (subprocess_create(process_args.data(), subprocess_option_no_window | subprocess_option_inherit_environment, &process_handle) != 0)
		{
			return command_result(command_result_failed_construct);
		}

		return command_result(std::move(process_handle));
	}

	std::filesystem::path get_global_git_path()
	{
		std::string_view env = std::getenv("PATH");

		while (!env.empty())
		{
			size_t separator_index = env.find(';');
			if (separator_index == env.npos)
			{
				separator_index = env.size();
			}

			std::filesystem::path path = env.substr(0, separator_index);

			#if defined(_WIN32)
			path /= "git.exe";
			#else
			path /= "git";
			#endif

			if (std::filesystem::is_regular_file(path))
			{
				return path;
			}

			env.remove_prefix(std::min(separator_index + 1, env.size()));
		}

		return std::filesystem::path();
	}
}
