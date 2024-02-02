#include "uuid.hpp"

namespace vt::utils::uuid
{
	uint64_t uuid::gen_uuid()
	{
		static std::random_device rd;
		std::mt19937_64 generator(rd());
		std::uniform_int_distribution<uint64_t> dis(1, std::numeric_limits<uint64_t>::max());

		return dis(generator);
	}
}
