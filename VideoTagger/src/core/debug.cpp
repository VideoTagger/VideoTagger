#include "debug.hpp"
#include <iostream>

namespace vt
{
	void debug::log(const std::string& message)
	{
		std::cerr << "Info: " << message << '\n';
	}

	void debug::error(const std::string& message)
	{
		std::cerr << "Error: " << message << '\n';
	}

	void debug::panic(const std::string& message)
	{
		std::cerr << "Panic!: " << message << '\n';
		std::abort();
	}
}
