#pragma once

namespace vt::utils
{
	template<typename type>
	constexpr type lerp(const type& start, const type& end, float alpha)
	{
		return type((1.f - alpha) * start + alpha * end);
	}
}
