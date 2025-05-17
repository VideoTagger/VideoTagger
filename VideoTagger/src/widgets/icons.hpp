#pragma once
#include <vector>
#include <string>

namespace vt::icons
{
	//https://fonts.google.com/icons?icon.set=Material+Symbols

	inline constexpr auto dots_hor = "\xee\x97\x93";
	inline constexpr auto play = "\xee\x80\xb7";
	inline constexpr auto play_next = "\xef\x9a\xb5";
	inline constexpr auto play_queue = "\xef\x9f\x90";
	inline constexpr auto pause = "\xee\x80\xb4";
	inline constexpr auto repeat = "\xee\x81\x80";
	inline constexpr auto repeat_one = "\xee\x81\x81";
	inline constexpr auto retry = "\xee\x81\x82";
	inline constexpr auto shuffle = "\xee\x81\x83";
	inline constexpr auto skip_next = "\xee\x81\x84";
	inline constexpr auto skip_prev = "\xee\x81\x85";
	inline constexpr auto fast_back = "\xee\x80\xa0";
	inline constexpr auto fast_fwd = "\xee\x80\x9f";
	inline constexpr auto save = "\xee\x85\xa1";
	inline constexpr auto save_as = "\xee\xad\xa0";
	inline constexpr auto import_ = "\xee\xa2\x9c";
	inline constexpr auto import_export = "\xee\x83\x83";
	inline constexpr auto exit = "\xee\x97\x8d";
	inline constexpr auto close = "\xee\xa1\xb9";
	inline constexpr auto back = "\xee\x97\x84";
	inline constexpr auto visibility_on = "\xee\xa3\xb4";
	inline constexpr auto visibility_off = "\xee\xa3\xb5";
	inline constexpr auto add = "\xee\x85\x85";
	inline constexpr auto delete_ = "\xee\xa1\xb2";
	inline constexpr auto reset = "\xef\x81\x93";
	inline constexpr auto folder = "\xee\x8b\x87";
	inline constexpr auto folder_code = "\xef\x8f\x88";
	inline constexpr auto file = "\xee\x99\xad";
	inline constexpr auto label = "\xee\xa2\x92";
	inline constexpr auto tags = "\xee\xa6\x8b";
	inline constexpr auto add_tag = "\xee\x98\x89";
	inline constexpr auto expand_less = "\xee\x97\x8e";
	inline constexpr auto expand_more = "\xee\x97\x8f";
	inline constexpr auto toggle_less = "\xee\x97\x96";
	inline constexpr auto toggle_more = "\xee\x97\x97";
	inline constexpr auto link = "\xee\x85\x97";
	inline constexpr auto link_off = "\xee\x85\xaf";
	inline constexpr auto edit = "\xee\x8f\x89";
	inline constexpr auto help = "\xee\xa3\xbd";
	inline constexpr auto video_group = "\xef\x92\x9a"; //"\xee\x81\x8a";
	inline constexpr auto video = "\xee\x80\xac";
	inline constexpr auto search = "\xee\xa2\xb6";
	inline constexpr auto terminal = "\xee\xae\x8e";
	inline constexpr auto download = "\xef\x82\x90";
	inline constexpr auto upload = "\xef\x82\x9b";
	inline constexpr auto download_off = "\xee\x93\xbe";
	inline constexpr auto arrow_up = "\xee\x97\x98";
	inline constexpr auto refresh = "\xee\x97\x95";
	inline constexpr auto local_storage = "\xee\x87\x9b";
	inline constexpr auto google_drive_add = "\xee\x99\x9c";
	inline constexpr auto home = "\xee\xa2\x8a";
	inline constexpr auto license = "\xee\xae\x81";
	inline constexpr auto translate = "\xee\xa3\xa2";

	inline constexpr auto property = "\xef\x8e\xaa";
	inline constexpr auto attribute = "\xee\xab\x93";
	inline constexpr auto object = "\xef\x9c\xa0";
	inline constexpr auto timeline = "\xef\x90\xb2";
	inline constexpr auto queue = "\xee\x81\xa4";
	inline constexpr auto database = "\xef\x88\x8e";
	
	inline constexpr auto bullet = "\xee\x81\xa1";
	inline constexpr auto chevron_left = "\xee\x97\x8b";
	inline constexpr auto chevron_right = "\xee\x97\x8c";

	inline constexpr auto shape = "\xee\x95\xb4";
	inline constexpr auto shape_none = "\xef\x80\xa3";
	inline constexpr auto shape_circle = "\xee\xbd\x8a";
	inline constexpr auto shape_rectangle = "\xee\x8f\x86";
	inline constexpr auto shape_polygon = "\xee\xad\x90";
	inline constexpr auto interpolate = "\xee\x9c\x9c";

	inline constexpr auto align_horizontal_left = "\xee\x80\x8d";
	inline constexpr auto align_horizontal_right = "\xee\x80\x90";
	inline constexpr auto align_horizontal_center = "\xee\x80\x8f";
	inline constexpr auto align_vertical_bottom = "\xee\x80\x95";
	inline constexpr auto align_vertical_top = "\xee\x80\x8c";
	inline constexpr auto align_vertical_center = "\xee\x80\x91";
	inline constexpr auto align_center = "\xee\x8d\x96";

	inline constexpr auto info = "\xee\xa2\x8e";
	inline constexpr auto warning = "\xee\x80\x82";
	//inline constexpr auto error = "\xee\x80\x80";
	inline constexpr auto error = "\xee\x85\xa0";
	inline constexpr auto exclamation = "\xee\x99\x85";
	inline constexpr auto delete_on_run = "\xee\x85\xac";

	inline constexpr auto keyframe = "\xee\x9a\x97";
	inline constexpr auto keyframe_current = "\xee\x9a\xb7";
	inline constexpr auto goto_keyframe = "\xee\x81\xb4";
	inline constexpr auto new_region = "\xef\x9f\x92";
	inline constexpr auto set_target = "\xee\xa2\x9f";

	inline std::vector<std::string> all
	({
		dots_hor, play, play_next, play_queue, pause, repeat, repeat_one, retry, shuffle, skip_next, skip_prev, fast_back, fast_fwd,
		save, save_as, import_, import_export, exit, close, back, visibility_on, visibility_off, add, delete_, reset, folder, folder_code, file, label, tags, add_tag, expand_less, expand_more,
		toggle_less, toggle_more, link, link_off, edit, help, video_group, video, search, terminal, property, attribute, object, timeline, queue, database,
		bullet, chevron_left, chevron_right,
		shape, shape_none, shape_circle, shape_rectangle, shape_polygon, interpolate,
		align_horizontal_left, align_horizontal_right, align_horizontal_center, align_vertical_bottom, align_vertical_top, align_vertical_center, align_center,
		info, warning, error, exclamation, delete_on_run,
		keyframe, keyframe_current, goto_keyframe, new_region, set_target,
		download, upload, download_off, arrow_up, refresh, local_storage, google_drive_add, home, license, translate
	});
}
