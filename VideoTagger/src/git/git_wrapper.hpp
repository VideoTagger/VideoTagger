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

	enum class command_status
	{
		running,
		finished
	};

	enum class file_status
	{
		unmodified,
		modified,
		added,
		deleted,
		renamed
	};

	struct file_list_item
	{
		file_list_item(std::filesystem::path name, file_status status, bool staged);

		std::filesystem::path name;
		file_status status;
		bool staged;
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

		std::optional<int> return_value();

		bool terminate();

		subprocess_s& process_handle();
		const subprocess_s& process_handle() const;

	private:
		std::unique_ptr<subprocess_s> process_handle_;
		std::optional<int> return_value_;
	};

	template<typename T>
	class promise
	{
	public:
		using value_type = T;

		explicit promise(execute_command_result&& command_result);

		T get();

		command_status status();

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

	class commit_result : public basic_result
	{
	public:
		explicit commit_result(execute_command_result& command_result);


	private:
		
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

	//TODO: validate command arguments ()
	class git_wrapper
	{
	public:
		git_wrapper(std::filesystem::path git_path, std::filesystem::path repository_path);

		path_status check_git_path() const;
		path_status check_repository_path() const;

		const std::filesystem::path& git_path() const;
		const std::filesystem::path& repository_path() const;

		promise<list_modified_files_result> list_modified_files();
		
		promise<commit_result> commit(const commit_arguments& arguments);

		execute_command_result execute_command(const std::string& command, const std::vector<std::string>& arguments) const;
	private:
		std::filesystem::path git_path_;
		std::filesystem::path repository_path_;
	};

	extern std::filesystem::path get_global_git_path();

	template<typename T>
	inline promise<T>::promise(execute_command_result&& command_result)
		: command_result_{ std::move(command_result) }
	{
	}

	template<typename T>
	inline T promise<T>::get()
	{
		return T(command_result_);
	}

	template<typename T>
	inline command_status promise<T>::status()
	{
		return command_result_.status();
	}
}
