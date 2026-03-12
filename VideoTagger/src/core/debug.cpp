#include "pch.hpp"
#include "debug.hpp"
#include <optional>
#include <core/app_context.hpp>

#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
	#include <Windows.h>
#endif

namespace vt
{
	debug::logging_mode debug::log_mode = debug::logging_mode::full;
	std::optional<std::thread::id> debug::main_thread_id{};

	void debug::init()
	{
		main_thread_id = std::this_thread::get_id();
#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		if (console == INVALID_HANDLE_VALUE) return;

		DWORD mode{};
		if (GetConsoleMode(console, &mode))
		{
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(console, mode);
		}
#endif
	}

	std::string debug::get_source()
	{
		if (!main_thread_id.has_value() or std::this_thread::get_id() == *main_thread_id)
		{
			return "main";
		}
		else
		{
			std::stringstream ss;
			ss << "thread:" << std::this_thread::get_id();
			return ss.str();
		}
	}

	std::filesystem::path debug::logs_filepath()
    {
        return ctx_.storage_path() / "logs";
    }
}
