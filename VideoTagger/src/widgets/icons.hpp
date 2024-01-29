#pragma once
#include <vector>

namespace vt::icons
{
	//https://fonts.google.com/icons?icon.set=Material+Icons

	inline constexpr auto dots_hor = u8"\xe5d3";
	inline constexpr auto play = u8"\xe037";
	inline constexpr auto pause = u8"\xe034";
	inline constexpr auto repeat = u8"\xe040";
	inline constexpr auto skip_next = u8"\xe044";
	inline constexpr auto skip_prev = u8"\xe045";
	inline constexpr auto fast_back = u8"\xe020";
	inline constexpr auto fast_fwd = u8"\xe01f";

	inline std::vector<std::string> all({ dots_hor, play, pause, repeat, skip_next, skip_prev, fast_back, fast_fwd });
}
