#pragma once
#include <filesystem>
#include <random>
#include <cstdint>

namespace vt::utils
{
	struct uuid
	{
		static uint64_t get();
	};
}
