#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <variant>

struct subprocess_s;

namespace vt::git
{
	enum class path_status
	{
		ok,
		not_found,
		wrong_file_type
	};

	enum class command_status
	{
		running,
		finished
	};

	enum class file_status
	{
		unknown,

		unmodified,
		modified,
		file_type_changed,
		added,
		deleted,
		renamed,
		copied
	};

	inline constexpr std::string_view file_status_to_string(file_status status);

	struct file_conflict_status
	{
		file_status local;
		file_status incoming;
	};

	struct file_list_item
	{
		file_list_item(std::filesystem::path name, std::variant<file_status, file_conflict_status> status, bool staged);

		std::filesystem::path name;
		bool staged;

		bool conflict() const;
		file_status& status();
		const file_status& status() const;
		file_conflict_status& conflict_status();
		const file_conflict_status& conflict_status() const;

	private:
		std::variant<file_status, file_conflict_status> status_;
	};

	struct command_result_failed_construct_t { explicit command_result_failed_construct_t() = default; };
	inline constexpr command_result_failed_construct_t command_result_failed_construct;

	class execute_command_result
	{
	public:
		execute_command_result(subprocess_s&& process_handle);
		execute_command_result(execute_command_result&&) = default;
		~execute_command_result();

		execute_command_result& operator=(execute_command_result&&) = default;

		command_status status();
		std::string read_stdout();
		std::string read_stderr();

		void wait();
		std::optional<int> return_value();

		bool terminate();

		subprocess_s& process_handle();
		const subprocess_s& process_handle() const;

	private:
		std::unique_ptr<subprocess_s> process_handle_;
		std::optional<int> return_value_;
	};

	template<typename T>
	class command_promise
	{
	public:
		using value_type = T;

		explicit command_promise(execute_command_result&& command_result);
		~command_promise();

		T get();

		command_status status();

		operator T();

	private:
		execute_command_result command_result_;
	};

	class basic_result
	{
	public:
		explicit basic_result(execute_command_result& command_result);

		int command_return_value() const;
		bool succeeded() const;

		const std::string& error_string();

	private:
		std::string error_string_;
		int command_return_value_;
	};

	struct commit_arguments
	{
		std::string message;
		std::string descryption;
	};

	class repository_path_result : basic_result
	{
	public:
		explicit repository_path_result(execute_command_result& command_result);

		std::filesystem::path path() const;

	private:
		std::filesystem::path path_;
	};

	class list_modified_files_result : basic_result
	{
	public:
		using iterator = std::vector<file_list_item>::iterator;
		using const_iterator = std::vector<file_list_item>::const_iterator;

		explicit list_modified_files_result(execute_command_result& command_result);

		file_list_item& at(size_t index);
		const file_list_item& at(size_t index) const;

		size_t size() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		std::vector<file_list_item> files_;
	};

	class is_repository_result : basic_result
	{
	public:
		explicit is_repository_result(execute_command_result& command_result);

		bool value() const;
		operator bool();

	private:
		bool value_;
	};
	

	//TODO: validate command arguments
	class git_wrapper
	{
	public:
		git_wrapper(std::filesystem::path git_path, std::filesystem::path working_directory);

		path_status check_git_path() const;
		path_status check_repository_path() const;

		void set_git_path(std::filesystem::path git_path);
		void set_working_directory(std::filesystem::path working_directory);

		const std::filesystem::path& git_path() const;
		const std::filesystem::path& working_directory() const;

		command_promise<repository_path_result> repository_path();
		command_promise<list_modified_files_result> list_modified_files();
		command_promise<is_repository_result> is_repository();
		
		command_promise<basic_result> init_repository();
		command_promise<basic_result> stage_files(const std::vector<std::filesystem::path>& files);
		command_promise<basic_result> unstage_files(const std::vector<std::filesystem::path>& files);
		command_promise<basic_result> commit(const commit_arguments& arguments);

		execute_command_result execute_command(const std::string& command, const std::vector<std::string>& arguments) const;
	private:
		std::filesystem::path git_path_;
		std::filesystem::path working_directory_;
	};

	extern std::filesystem::path get_global_git_path();

	inline constexpr std::string_view file_status_to_string(file_status status)
	{
		switch (status)
		{
		case vt::git::file_status::unmodified:			return "unmodified";
		case vt::git::file_status::modified:			return "modified";
		case vt::git::file_status::file_type_changed:	return "file_type_changed";
		case vt::git::file_status::added:				return "added";
		case vt::git::file_status::deleted:				return "deleted";
		case vt::git::file_status::renamed:				return "renamed";
		case vt::git::file_status::copied:				return "copied";
		default:										return "unknown";
		}
	}

	template<typename T>
	inline command_promise<T>::command_promise(execute_command_result&& command_result)
		: command_result_{ std::move(command_result) }
	{
	}

	template<typename T>
	inline command_promise<T>::~command_promise()
	{
		command_result_.wait();
	}

	template<typename T>
	inline T command_promise<T>::get()
	{
		command_result_.wait();
		return T(command_result_);
	}

	template<typename T>
	inline command_status command_promise<T>::status()
	{
		return command_result_.status();
	}

	template<typename T>
	inline command_promise<T>::operator T()
	{
		return get();
	}
}