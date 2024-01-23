#include "debug.hpp"
#include <iostream>
#include <fstream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#ifdef _WIN32
	#include <Windows.h>
#endif

namespace vt
{
	debug::logging_mode debug::log_mode = debug::logging_mode::full;

	void debug::init()
	{
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

	void debug::log(const std::string& message)
	{
		constexpr auto flag = "[Info]";
		std::string msg = ": " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << "\033[0;36m" << flag << "\033[0m" << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << flag << msg;
		}
	}

	void debug::error(const std::string& message)
	{
		constexpr auto flag = "[Error]";
		std::string msg = ": " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << "\033[31m" << flag << "\033[0m" << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << msg;
		}
	}

	void debug::panic(const std::string& message)
	{
		constexpr auto flag = "[Panic!]";
		std::string msg = ": " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << "\033[0;91m" << flag << "\033[0m" << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << flag << msg;
		}
		std::abort();
	}
}
