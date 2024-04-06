#pragma once
#include <string>
#include <filesystem>

namespace vt
{
	struct debug
	{
		enum class logging_mode : uint8_t
		{
			none = 1 << 0,
			console = 1 << 1,
			file = 1 << 2,
			full = console | file
		};

		static constexpr const char* log_filepath = "app.log";
		static logging_mode log_mode;

		static void init();
		//Logs the message with an 'Info' flag
		template<typename... args_t>
		static void log(fmt::format_string<args_t...> format, args_t&&... args)
		{
			constexpr auto flag = "[Info]";
			std::string msg = ": " + fmt::vformat(format.get(), fmt::make_format_args(args...)) + '\n';
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
		//Logs the message with an 'Error' flag
		template<typename... args_t>
		static void error(fmt::format_string<args_t...> format, args_t&&... args)
		{
			constexpr auto flag = "[Error]";
			std::string msg = ": " + fmt::vformat(format.get(), fmt::make_format_args(args...)) + '\n';
			if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
			{
				std::cerr << "\033[31m" << flag << "\033[0m" << msg;
			}
			if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
			{
				std::ofstream log(log_filepath, std::ios::app);
				if (log.is_open()) log << flag << msg;
			}
		}
		//Logs the message with a 'Panic!' flag and immediately shuts down the app
		template<typename... args_t>
		static void panic(fmt::format_string<args_t...> format, args_t&&... args)
		{
			constexpr auto flag = "[Panic!]";
			std::string msg = ": " + fmt::vformat(format.get(), fmt::make_format_args(args...)) + '\n';
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
	};
}
