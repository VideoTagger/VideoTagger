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

	std::string execute_command_result::read_stdout()
	{
		if (status() != command_status::finished)
		{
			return std::string();
		}

		return read_FILE(subprocess_stdout(process_handle_.get()));
	}

	std::string execute_command_result::read_stderr()
	{
		if (status() != command_status::finished)
		{
			return std::string();
		}

		return read_FILE(subprocess_stderr(process_handle_.get()));
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

	basic_result::basic_result(execute_command_result& command_result)
	{
		command_return_value_ = *command_result.return_value();
		error_string_ = command_result.read_stderr();
	}

	int basic_result::command_return_value() const
	{
		return command_return_value_;
	}

	bool basic_result::succeeded() const
	{
		return command_return_value_ == 0;
	}

	const std::string& basic_result::error_string()
	{
		return error_string_;
	}

	repository_path_result::repository_path_result(execute_command_result& command_result)
		: basic_result(command_result)
	{
		std::string string_path = command_result.read_stdout();
		//remove new line
		string_path.pop_back();
		path_ = string_path;
	}

	std::filesystem::path repository_path_result::path() const
	{
		return path_;
	}

	list_modified_files_result::list_modified_files_result(execute_command_result& command_result)
		: basic_result(command_result)
	{

		std::string command_output = command_result.read_stdout();

		auto func = [this](std::string_view line)
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
			files_.emplace_back(std::filesystem::path(line), status, staged);
		};

		vt::utils::string::for_each_line(command_output, func);
	}

	file_list_item& list_modified_files_result::at(size_t index)
	{
		return files_.at(index);
	}

	const file_list_item& list_modified_files_result::at(size_t index) const
	{
		return files_.at(index);
	}

	size_t list_modified_files_result::size() const
	{
		return files_.size();
	}

	list_modified_files_result::iterator list_modified_files_result::begin()
	{
		return files_.begin();
	}

	list_modified_files_result::const_iterator list_modified_files_result::begin() const
	{
		return files_.begin();
	}

	list_modified_files_result::const_iterator list_modified_files_result::cbegin() const
	{
		return files_.cbegin();
	}

	list_modified_files_result::iterator list_modified_files_result::end()
	{
		return files_.end();
	}

	list_modified_files_result::const_iterator list_modified_files_result::end() const
	{
		return files_.end();
	}

	list_modified_files_result::const_iterator list_modified_files_result::cend() const
	{
		return files_.cend();
	}

	is_repository_result::is_repository_result(execute_command_result& command_result)
		: basic_result(command_result)
	{
		if (!succeeded())
		{
			value_ = false;
			return;
		}

		std::string output = command_result.read_stdout();
		output.pop_back();

		if (output == "true")
		{
			value_ = true;
		}
		else
		{
			value_ = false;
		}
	}

	bool is_repository_result::value() const
	{
		return value_;
	}

	is_repository_result::operator bool()
	{
		return value_;
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

	promise<repository_path_result> git_wrapper::repository_path()
	{
		std::vector<std::string> command_arguments{
			"--show-toplevel"
		};

		return promise<repository_path_result>(execute_command("rev-parse", command_arguments));
	}

	promise<list_modified_files_result> git_wrapper::list_modified_files()
	{
		std::vector<std::string> command_arguments{
			"--porcelain",
			"-uall"
		};

		return promise<list_modified_files_result>(execute_command("status", command_arguments));
	}

	promise<is_repository_result> git_wrapper::is_repository()
	{
		std::vector<std::string> command_arguments{
			"--is-inside-work-tree",
		};

		return promise<is_repository_result>(execute_command("rev-parse", command_arguments));
	}

	promise<basic_result> git_wrapper::init_repository()
	{
		return promise<basic_result>(execute_command("init", {}));
	}

	promise<basic_result> git_wrapper::stage_files(const std::vector<std::filesystem::path>& files)
	{
		std::vector<std::string> command_arguments;
		command_arguments.reserve(files.size());
		for (auto& path : files)
		{
			command_arguments.push_back(path.u8string());
		}

		return promise<basic_result>(execute_command("add", command_arguments));
	}

	promise<basic_result> git_wrapper::unstage_files(const std::vector<std::filesystem::path>& files)
	{
		std::vector<std::string> command_arguments;
		command_arguments.reserve(files.size() + 1);
		command_arguments.push_back("--staged");
		for (auto& path : files)
		{
			command_arguments.push_back(path.u8string());
		}

		return promise<basic_result>(execute_command("restore", command_arguments));
	}

	promise<basic_result> git_wrapper::commit(const commit_arguments& arguments)
	{
		std::vector<std::string> command_arguments{
			"-m", arguments.message.c_str(),
			"-m", arguments.descryption.c_str()
		};

		return promise<basic_result>(execute_command("commit", command_arguments));
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
