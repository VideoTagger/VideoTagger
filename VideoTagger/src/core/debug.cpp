#include "pch.hpp"
#include "debug.hpp"

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
}
