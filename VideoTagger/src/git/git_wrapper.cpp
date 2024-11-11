#include "pch.hpp"
#include <cstdlib>
#include "git_wrapper.hpp"
#include <core/debug.hpp>
#include <utils/string.hpp>

#include "subprocess.hpp"

namespace vt::git
{
	file_list_item::file_list_item(std::filesystem::path name, std::variant<file_status, file_conflict_status> status, bool staged)
		: name{ name }, status_{ status }, staged{ staged }
	{
	}

	bool file_list_item::conflict() const
	{
		return std::holds_alternative<file_conflict_status>(status_);
	}

	file_status& file_list_item::status()
	{
		return std::get<file_status>(status_);
	}

	const file_status& file_list_item::status() const
	{
		return std::get<file_status>(status_);
	}

	file_conflict_status& file_list_item::conflict_status()
	{
		return std::get<file_conflict_status>(status_);
	}

	const file_conflict_status& file_list_item::conflict_status() const
	{
		return std::get<file_conflict_status>(status_);
	}

	execute_command_result::execute_command_result(subprocess_s&& process_handle)
		: process_handle_{ std::make_unique<subprocess_s>() }
	{
		*process_handle_ = process_handle;
		std::memset(&process_handle, 0, sizeof(subprocess_s));
	}

	execute_command_result::~execute_command_result()
	{
		if (process_handle_ != nullptr)
		{
			subprocess_destroy(process_handle_.get());
		}
	}

	command_status execute_command_result::status()
	{
		if (subprocess_alive(process_handle_.get()))
		{
			return command_status::running;
		}

		return command_status::finished;
	}

	static void read_FILE(FILE* file, std::ostream& out)
	{
		std::array<char, 4096> read_buffer{};

		size_t bytes_read{};
		while (bytes_read = fread(read_buffer.data(), 1, read_buffer.size(), file))
		{
			out.write(read_buffer.data(), bytes_read);
		}
	}

	std::string execute_command_result::read_stdout()
	{
		if (status() != command_status::finished)
		{
			return std::string();
		}

		std::stringstream ss;
		read_FILE(subprocess_stdout(process_handle_.get()), ss);
		return ss.str();
	}

	bool execute_command_result::read_stdout_to_file(const std::filesystem::path& file_path)
	{
		if (status() != command_status::finished)
		{
			return false;
		}

		std::ofstream file(file_path);
		if (!file.is_open())
		{
			return false;
		}
		read_FILE(subprocess_stdout(process_handle_.get()), file);
		return true;
	}

	std::string execute_command_result::read_stderr()
	{
		if (status() != command_status::finished)
		{
			return std::string();
		}

		std::stringstream ss;
		read_FILE(subprocess_stderr(process_handle_.get()), ss);
		return ss.str();
	}

	bool execute_command_result::read_stderr_to_file(const std::filesystem::path& file_path)
	{
		if (status() != command_status::finished)
		{
			return false;
		}

		std::ofstream file(file_path);
		if (!file.is_open())
		{
			return false;
		}
		read_FILE(subprocess_stderr(process_handle_.get()), file);
		return true;
	}

	void execute_command_result::wait()
	{
		static_cast<void>(return_value());
	}

	std::optional<int> execute_command_result::return_value()
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

	bool execute_command_result::terminate()
	{
		if (status() != command_status::running)
		{
			return true;
		}

		return subprocess_terminate(process_handle_.get());
	}

	subprocess_s& execute_command_result::process_handle()
	{
		return *process_handle_;
	}

	const subprocess_s& execute_command_result::process_handle() const
	{
		return *process_handle_;
	}

	generic_command_error_result::generic_command_error_result(std::string message, int return_value)
		: message_{ std::move(message) }, return_value_{ return_value }
	{
	}

	int generic_command_error_result::return_value() const
	{
		return return_value_;
	}

	const std::string& generic_command_error_result::message() const
	{
		return message_;
	}

	template<>
	extern version_result make_command_result<version_result>(execute_command_result& command_result)
	{
		if (auto error = make_generic_error<version_result::error_type>(command_result); error.has_value())
		{
			return version_result(std::move(*error));
		}

		auto parts = utils::string::split(command_result.read_stdout(), ' ');

		version_result::error_type error("Unexpected output", -1);

		if (parts.size() != 3)
		{
			return version_result(error);
		}
		if (parts[0] != "git" or parts[1] != "version")
		{
			return version_result(error);
		}

		auto version_parts = utils::string::split(parts[2], '.');
		if (version_parts.size() < 3)
		{
			return version_result(error);
		}

		version_result::value_type version;
		version.major = std::stoi(version_parts[0]);
		version.minor = std::stoi(version_parts[1]);
		version.patch = std::stoi(version_parts[2]);
		return version_result(version);
	}

	template<>
	extern list_modified_files_result make_command_result<list_modified_files_result>(execute_command_result& command_result)
	{
		if (auto error = make_generic_error<list_modified_files_result::error_type>(command_result); error.has_value())
		{
			return list_modified_files_result(std::move(*error));
		}

		std::string command_output = command_result.read_stdout();
		list_modified_files_result::value_type result;

		auto func = [&result](std::string_view line)
			{
				auto char_to_status = [](char ch)
					{
						switch (ch)
						{
						case ' ':	return file_status::unmodified;
						case 'M':	return file_status::modified;
						case 'T':	return file_status::file_type_changed;
						case 'A':	return file_status::added;
						case 'D':	return file_status::deleted;
						case 'R':	return file_status::renamed;
						case 'C':	return file_status::copied;
						case 'U':	return file_status::modified;
						default:	return file_status::unknown;
						}
					};

				char status_char{};

				std::variant<file_status, file_conflict_status> status{};
				bool staged{};

				if (line.substr(0, 2) == "??")
				{
					staged = false;
					status = file_status::added;
				}
				else if (line[0] == ' ' or line[1] == ' ')
				{
					if (line[0] == ' ')
					{
						staged = false;
						status_char = line[1];
					}
					else
					{
						staged = true;
						status_char = line[0];
					}

					status = char_to_status(status_char);
				}
				else
				{
					status = file_conflict_status{ char_to_status(line[0]), char_to_status(line[1]) };
					staged = false;
				}

				line.remove_prefix(3);
				result.emplace_back(std::filesystem::path(line), status, staged);
			};

		vt::utils::string::for_each_line(command_output, func);
		return list_modified_files_result(result);
	}

	template<>
	extern bool_result make_command_result<bool_result>(execute_command_result& command_result)
	{
		std::optional<int> command_return = command_result.return_value();
		if (!command_return.has_value())
		{
			return bool_result(bool_result::error_type("Proccess failure", -1));
		}

		if (*command_return != 0)
		{
			return bool_result(false);
		}

		std::string output = command_result.read_stdout();
		if (!output.empty())
		{
			output.pop_back();
			if (output == "true" and output.back() == '\n')
			{
				return bool_result(true);
			}
		}

		return bool_result(false);
	}
	
	git_wrapper::git_wrapper(std::filesystem::path git_path, std::filesystem::path working_directory)
		: git_path_{ git_path }, working_directory_{ working_directory }
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

		//TODO: find some better way to check if it's actually git
		auto result = version().get();
		if (!result.has_value())
		{
			return path_status::not_git;
		}

		return path_status::ok;
	}

	path_status git_wrapper::check_repository_path() const
	{
		std::filesystem::file_status file_status = std::filesystem::status(working_directory_);

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

	void git_wrapper::set_git_path(std::filesystem::path git_path)
	{
		git_path_ = std::move(git_path);
	}

	void git_wrapper::set_working_directory(std::filesystem::path working_directory)
	{
		working_directory_ = std::move(working_directory);
	}

	const std::filesystem::path& git_wrapper::git_path() const
	{
		return git_path_;
	}

	const std::filesystem::path& git_wrapper::working_directory() const
	{
		return working_directory_;
	}

	command_promise<version_result> git_wrapper::version() const
	{
		return command_promise<version_result>(execute_command("version", {}));
	}

	command_promise<path_result> git_wrapper::repository_path() const
	{
		std::vector<std::string> command_arguments{
			"--show-toplevel"
		};

		return command_promise<path_result>(execute_command("rev-parse", command_arguments));
	}

	command_promise<list_modified_files_result> git_wrapper::list_modified_files() const
	{
		std::vector<std::string> command_arguments{
			"--porcelain",
			"-uall"
		};

		return command_promise<list_modified_files_result>(execute_command("status", command_arguments));
	}

	command_promise<bool_result> git_wrapper::is_repository() const
	{
		std::vector<std::string> command_arguments{
			"--is-inside-work-tree",
		};

		return command_promise<bool_result>(execute_command("rev-parse", command_arguments));
	}

	command_promise<string_result> git_wrapper::current_branch() const
	{
		std::vector<std::string> command_arguments{
			"--show-current",
		};

		return command_promise<string_result>(execute_command("branch", command_arguments));
	}

	command_promise<void_result> git_wrapper::init_repository() const
	{
		return command_promise<void_result>(execute_command("init", {}));
	}

	command_promise<void_result> git_wrapper::clone(const std::string& remote_url, const clone_arguments& arguments) const
	{
		std::vector<std::string> command_arguments;
		if (arguments.no_checkout)
		{
			command_arguments.push_back("--no-checkout");
		}

		command_arguments.push_back(remote_url);

		if (arguments.clone_into)
		{
			command_arguments.push_back(arguments.clone_into->u8string());
		}

		return command_promise<void_result>(execute_command("clone", command_arguments));
	}

	command_promise<void_result> git_wrapper::stage_files(const std::vector<std::filesystem::path>& files) const
	{
		std::vector<std::string> command_arguments;
		command_arguments.reserve(files.size());
		for (auto& path : files)
		{
			command_arguments.push_back(path.u8string());
		}

		return command_promise<void_result>(execute_command("add", command_arguments));
	}

	command_promise<void_result> git_wrapper::unstage_files(const std::vector<std::filesystem::path>& files) const
	{
		std::vector<std::string> command_arguments;
		command_arguments.reserve(files.size() + 1);
		command_arguments.push_back("--staged");
		for (auto& path : files)
		{
			command_arguments.push_back(path.u8string());
		}

		return command_promise<void_result>(execute_command("restore", command_arguments));
	}

	command_promise<void_result> git_wrapper::commit(const std::string& message, const commit_arguments& arguments) const
	{
		std::vector<std::string> command_arguments{
			"-m", message.c_str(),
			"-m", arguments.descryption.c_str()
		};

		return command_promise<void_result>(execute_command("commit", command_arguments));
	}

	execute_command_result git_wrapper::execute_command(const std::string& command, const std::vector<std::string>& arguments) const
	{
		std::vector<const char*> process_args;
		process_args.reserve((4 + arguments.size() + 1));
		
		std::string git_path_string = git_path_.u8string();
		std::string rep_path_string = working_directory_.u8string();

		process_args.push_back(git_path_string.c_str());
		process_args.push_back("-C");
		process_args.push_back(rep_path_string.c_str());

		process_args.push_back(command.c_str());

		for (const auto& arg : arguments)
		{
			if (arg.empty())
			{
				continue;
			}

			process_args.push_back(arg.c_str());
		}

		process_args.push_back(nullptr);

		subprocess_s process_handle{};
		if (subprocess_create(process_args.data(), subprocess_option_no_window | subprocess_option_inherit_environment, &process_handle) != 0)
		{
			//TODO: maybe do something else, the file could just be missing
			process_args.pop_back();
			debug::panic("Failed to create subprocess with args: {}", fmt::join(process_args, " "));
		}

		return execute_command_result(std::move(process_handle));
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
