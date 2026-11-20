#pragma once
#include <stdexcept>

namespace vt
{
	class interrupt_exception : public std::runtime_error
	{
	public:
		interrupt_exception(const std::string& message) : std::runtime_error{ message } {}
	};
}
