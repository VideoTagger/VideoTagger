#pragma once
#include <vector>

namespace vt::icons
{
	//https://fonts.google.com/icons?icon.set=Material+Icons

	inline constexpr auto dots_hor = "\xee\x97\x93";
	inline constexpr auto play = "\xee\x80\xb7";
	inline constexpr auto pause = "\xee\x80\xb4";
	inline constexpr auto repeat = "\xee\x81\x80";
	inline constexpr auto skip_next = "\xee\x81\x84";
	inline constexpr auto skip_prev = "\xee\x81\x85";
	inline constexpr auto fast_back = "\xee\x80\xa0";
	inline constexpr auto fast_fwd = "\xee\x80\x9f";
	inline constexpr auto save = "\xee\x85\xa1";
	inline constexpr auto save_as = "\xee\xad\xa0";
	inline constexpr auto close = "\xee\x97\x8d";
	inline constexpr auto visibility_on = "\xee\xa3\xb4";
	inline constexpr auto visibility_off = "\xee\xa3\xb5";
	inline constexpr auto delete_ = "\xee\xa1\xb2";
	inline constexpr auto folder = "\xee\x8b\x87";
	inline constexpr auto label = "\xee\xa2\x92";
	inline constexpr auto expand_less = "\xee\x97\x8e";
	inline constexpr auto expand_more = "\xee\x97\x8f";
	inline constexpr auto link = "\xee\x85\x97";

	inline std::vector<std::string> all
	({
		dots_hor, play, pause, repeat, skip_next, skip_prev, fast_back, fast_fwd,
		save, save_as, close, visibility_on, visibility_off, delete_, folder, label, expand_less, expand_more, link
	});
}
