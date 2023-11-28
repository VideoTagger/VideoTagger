#pragma once
#include <string>

namespace vt
{
	struct debug
	{
		//Logs the message with an 'Info' flag
		static void log(const std::string& message);
		//Logs the message with an 'Error' flag
		static void error(const std::string& message);
		//Logs the message with a 'Panic!' flag and immediately shuts down the app
		static void panic(const std::string& message);
	};
}
