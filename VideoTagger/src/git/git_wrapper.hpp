#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <type_traits>

struct subprocess_s;

namespace vt::git
{
	enum class path_status
	{
		ok,
		not_found,
		wrong_file_type,
		not_git
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

	struct git_version
	{
		int major{};
		int minor{};
		int patch{};
	};

	class execute_command_result
	{
	public:
		execute_command_result(subprocess_s&& process_handle);
		execute_command_result(execute_command_result&&) = default;
		~execute_command_result();

		execute_command_result& operator=(execute_command_result&&) = default;

		command_status status();
		std::string read_stdout();
		bool read_stdout_to_file(const std::filesystem::path& file_path);
		std::string read_stderr();
		bool read_stderr_to_file(const std::filesystem::path& file_path);

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
	
	class generic_command_error_result
	{
	public:
		explicit generic_command_error_result(std::string message, int return_value);

		int return_value() const;
		const std::string& message() const;

	private:
		std::string message_;
		int return_value_;
	};

	template<typename T, typename E>
	class basic_command_result
	{
	public:
		using value_type = T;
		using error_type = E;

		basic_command_result(value_type value);
		basic_command_result(error_type error);

		constexpr T& value() &;
		constexpr const T& value() const&;
		constexpr T&& value() &&;
		constexpr const T&& value() const&&;

		constexpr E& error() &;
		constexpr const E& error() const&;
		constexpr E&& error() &&;
		constexpr const E&& error() const&&;

		bool has_value() const;
		operator bool() const;

	private:
		std::variant<T, E> value_error_;
	};

	template<typename E>
	class basic_command_result<void, E>
	{
	public:
		using value_type = void;
		using error_type = E;

		basic_command_result();
		basic_command_result(error_type error);
		
		void value() const;

		constexpr E& error()&;
		constexpr const E& error() const&;
		constexpr E&& error()&&;
		constexpr const E&& error() const&&;

		bool has_value() const;
		operator bool() const;

	private:
		std::variant<std::monostate, E> value_error_;
	};

	template<typename ResultType>
	inline ResultType make_command_result(execute_command_result& command_result);

	template<typename E>
	inline std::optional<E> make_generic_error(execute_command_result& command_result);

	struct commit_arguments
	{
		std::string descryption;
	};

	struct clone_arguments
	{
		std::optional<std::filesystem::path> clone_into;
		bool no_checkout = false;
	};
	
	using version_result = basic_command_result<git_version, generic_command_error_result>;
	template<> 
	extern version_result make_command_result<version_result>(execute_command_result& command_result);

	using path_result = basic_command_result<std::filesystem::path, generic_command_error_result>;

	using list_modified_files_result = basic_command_result<std::vector<file_list_item>, generic_command_error_result>;
	template<>
	extern list_modified_files_result make_command_result<list_modified_files_result>(execute_command_result& command_result);

	using bool_result = basic_command_result<bool, generic_command_error_result>;
	template<>
	extern bool_result make_command_result<bool_result>(execute_command_result& command_result);

	using string_result = basic_command_result<std::string, generic_command_error_result>;

	using void_result = basic_command_result<void, generic_command_error_result>;

	//TODO: validate command arguments
	class git_wrapper
	{
	public:
		git_wrapper() = default;
		git_wrapper(std::filesystem::path git_path, std::filesystem::path working_directory);

		path_status check_git_path() const;
		path_status check_repository_path() const;

		void set_git_path(std::filesystem::path git_path);
		void set_working_directory(std::filesystem::path working_directory);

		const std::filesystem::path& git_path() const;
		const std::filesystem::path& working_directory() const;

		command_promise<version_result> version() const;
		command_promise<path_result> repository_path() const;
		command_promise<list_modified_files_result> list_modified_files() const;
		command_promise<bool_result> is_repository() const;
		command_promise<string_result> current_branch() const;
		
		command_promise<void_result> init_repository() const;
		command_promise<void_result> clone(const std::string& remote_url, const clone_arguments& arguments) const;
		command_promise<void_result> stage_files(const std::vector<std::filesystem::path>& files) const;
		command_promise<void_result> unstage_files(const std::vector<std::filesystem::path>& files) const;
		command_promise<void_result> commit(const std::string& message, const commit_arguments& arguments) const;

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
		return make_command_result<T>(command_result_);
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

	template<typename T, typename E>
	inline basic_command_result<T, E>::basic_command_result(value_type value)
		: value_error_{ std::move(value) }
	{
	}

	template<typename T, typename E>
	inline basic_command_result<T, E>::basic_command_result(error_type error)
		: value_error_{ std::move(error) }
	{
	}

	template<typename T, typename E>
	inline constexpr T& basic_command_result<T, E>::value() &
	{
		return std::get<T>(value_error_);
	}

	template<typename T, typename E>
	inline constexpr const T& basic_command_result<T, E>::value() const&
	{
		return std::get<T>(value_error_);
	}

	template<typename T, typename E>
	inline constexpr T&& basic_command_result<T, E>::value() &&
	{
		return std::move(std::get<T>(value_error_));
	}

	template<typename T, typename E>
	inline constexpr const T&& basic_command_result<T, E>::value() const&&
	{
		return std::move(std::get<T>(value_error_));
	}

	template<typename T, typename E>
	inline constexpr E& basic_command_result<T, E>::error() &
	{
		return std::get<E>(value_error_);
	}

	template<typename T, typename E>
	inline constexpr const E& basic_command_result<T, E>::error() const&
	{
		return std::get<E>(value_error_);
	}

	template<typename T, typename E>
	inline constexpr E&& basic_command_result<T, E>::error() &&
	{
		return std::move(std::get<error_type>(value_error_));
	}

	template<typename T, typename E>
	inline constexpr const E&& basic_command_result<T, E>::error() const&&
	{
		return std::move(std::get<error_type>(value_error_));
	}

	template<typename T, typename E>
	inline bool basic_command_result<T, E>::has_value() const
	{
		return std::holds_alternative<T>(value_error_);
	}

	template<typename T, typename E>
	inline basic_command_result<T, E>::operator bool() const
	{
		return has_value();
	}

	template<typename E>
	inline basic_command_result<void, E>::basic_command_result()
		: value_error_{ std::monostate{} }
	{
	}

	template<typename E>
	inline basic_command_result<void, E>::basic_command_result(error_type error)
		: value_error_{ std::move(error) }
	{
	}

	template<typename E>
	inline void basic_command_result<void, E>::value() const
	{
	}

	template<typename E>
	inline constexpr E& basic_command_result<void, E>::error() &
	{
		return std::get<E>(value_error_);
	}

	template<typename E>
	inline constexpr const E& basic_command_result<void, E>::error() const&
	{
		return std::get<E>(value_error_);
	}

	template<typename E>
	inline constexpr E&& basic_command_result<void, E>::error()&&
	{
		return std::move(std::get<E>(value_error_));
	}

	template<typename E>
	inline constexpr const E&& basic_command_result<void, E>::error() const&&
	{
		return std::move(std::get<E>(value_error_));
	}

	template<typename E>
	inline bool basic_command_result<void, E>::has_value() const
	{
		return std::holds_alternative<std::monostate>(value_error_);
	}

	template<typename E>
	inline basic_command_result<void, E>::operator bool() const
	{
		return has_value();
	}

	template<typename ResultType>
	inline ResultType make_command_result(execute_command_result& command_result)
	{
		if (auto error = make_generic_error<typename ResultType::error_type>(command_result); error.has_value())
		{
			return ResultType(std::move(*error));
		}

		if constexpr (std::is_same_v<typename ResultType::value_type, void>)
		{
			return ResultType();
		}
		else
		{
			std::string result = command_result.read_stdout();
			if (!result.empty() and result.back() == '\n')
			{
				result.pop_back();
			}

			return ResultType(typename ResultType::value_type(result));
		}
	}

	template<typename E>
	std::optional<E> make_generic_error(execute_command_result& command_result)
	{
		std::optional<int> command_return = command_result.return_value();
		if (!command_return.has_value())
		{
			return std::optional<E>{ E("Proccess failure", -1) };
		}

		if (*command_return != 0)
		{
			return std::optional<E>{ E(command_result.read_stderr(), *command_result.return_value()) };
		}

		return std::nullopt;
	}
}
