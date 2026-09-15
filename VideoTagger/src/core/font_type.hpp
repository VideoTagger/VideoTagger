#pragma once
#include <cstdint>

namespace vt
{
	enum class font_type : uint8_t
	{
		h1,
		h1_bold,
		h2,
		h2_bold,
		h3,
		h3_bold,
		h4,
		h5,
		h6,
		thumbnail,
		password,
		normal = h4,
	};
}
