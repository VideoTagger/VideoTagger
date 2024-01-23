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
		static void log(const std::string& message);
		//Logs the message with an 'Error' flag
		static void error(const std::string& message);
		//Logs the message with a 'Panic!' flag and immediately shuts down the app
		static void panic(const std::string& message);
	};
}
