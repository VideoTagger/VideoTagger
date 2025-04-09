#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <utils/time.hpp>

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

		static logging_mode log_mode;

		static void init();
		template<typename... args_t>
		static void log_source(const std::string& source, std::string flag, const fmt::format_string<args_t...> format, args_t&&... args)
		{
			std::string color;
			if (flag == "Info")
			{
				color = "\033[0;36m";
			}
			else if (flag == "Warn")
			{
				color = "\033[0;33m";
			}
			else if (flag == "Error")
			{
				color = "\033[31m";
			}
			else if (flag == "Panic!")
			{
				color = "\033[0;91m";
			}

			std::string colored_flag = fmt::format("{}[{}]\033[0m({})", color, flag, source);
			flag = fmt::format("[{}]({})", flag, source);
			auto ts = utils::time::utc_timestamp();
			auto colored_timestamp = fmt::format("\033[0;90m[{}]", ts);
			auto timestamp = fmt::format("[{}]", ts);
			std::string msg = ": " + fmt::vformat(format.get(), fmt::make_format_args(args...)) + '\n';
			if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
			{
				std::cerr << colored_timestamp << colored_flag << msg;
			}
			if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
			{
				auto logs_dir = logs_filepath();
				if (!std::filesystem::exists(logs_dir))
				{
					std::filesystem::create_directories(logs_dir);
				}

				auto log_filepath = logs_dir / "app.log";

				std::ofstream log(log_filepath, std::ios::app);
				if (log.is_open()) log << timestamp << flag << msg;
			}
		}

		//Logs the message with an 'Info' flag
		template<typename... args_t>
		static void log(const fmt::format_string<args_t...> format, args_t&&... args)
		{
			log_source("main", "Info", format, std::forward<args_t&&>(args)...);
		}

		//Logs the message with an 'Warn' flag
		template<typename... args_t>
		static void warn(const fmt::format_string<args_t...> format, args_t&&... args)
		{
			log_source("main", "Warn", format, std::forward<args_t&&>(args)...);
		}

		//Logs the message with an 'Error' flag
		template<typename... args_t>
		static void error(const fmt::format_string<args_t...> format, args_t&&... args)
		{
			log_source("main", "Error", format, std::forward<args_t&&>(args)...);
		}
		//Logs the message with a 'Panic!' flag and immediately shuts down the app
		template<typename... args_t>
		static void panic(const fmt::format_string<args_t...> format, args_t&&... args)
		{
			log_source("main", "Panic!", format, std::forward<args_t&&>(args)...);
		}

		static std::filesystem::path logs_filepath();
	};
}
