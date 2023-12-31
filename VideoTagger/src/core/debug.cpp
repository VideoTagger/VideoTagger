#include "debug.hpp"
#include <iostream>
#include <fstream>

namespace vt
{
	debug::logging_mode debug::log_mode = debug::logging_mode::full;

	void debug::log(const std::string& message)
	{
		std::string msg = "[Info]: " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << msg;
		}
	}

	void debug::error(const std::string& message)
	{
		std::string msg = "[Error]: " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << msg;
		}
	}

	void debug::panic(const std::string& message)
	{
		std::string msg = "[Panic!]: " + message + '\n';
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::console))
		{
			std::cerr << msg;
		}
		if (static_cast<uint8_t>(log_mode) & static_cast<uint8_t>(debug::logging_mode::file))
		{
			std::ofstream log(log_filepath, std::ios::app);
			if (log.is_open()) log << msg;
		}
		std::abort();
	}
}
