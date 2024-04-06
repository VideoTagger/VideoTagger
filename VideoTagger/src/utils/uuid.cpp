#include "pch.hpp"
#include "uuid.hpp"

namespace vt::utils
{
	uint64_t uuid::get()
	{
		static std::random_device rd;
		static std::mt19937_64 generator(rd());
		static std::uniform_int_distribution<uint64_t> distribution(1, std::numeric_limits<uint64_t>::max());

		return distribution(generator);
	}
}
