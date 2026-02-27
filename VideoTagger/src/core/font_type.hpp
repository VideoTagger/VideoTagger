#pragma once
#include <cstdint>

namespace vt
{
	enum class font_type : uint8_t
	{
		h1,
		h2,
		h3,
		h4,
		h5,
		h6,
		thumbnail,
		normal = h4,
	};
}
