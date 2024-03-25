#pragma once
#include <filesystem>
#include <random>
#include <cstdint>

namespace vt::utils
{
	struct uuid
	{
		uuid() = delete;
		static uint64_t get();
	};
}
