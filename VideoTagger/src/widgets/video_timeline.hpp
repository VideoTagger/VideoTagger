// THIS IS A MODIFED VERSION OF THE SEQUENCER WIDGET FROM ImGuizmo
// 
// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include <utils/timestamp.hpp>
#include <tags/tag_storage.hpp>
#include <tags/tag_timeline.hpp>
#include <video/video_pool.hpp>

namespace vt::widgets
{
	//TODO: maybe should be moved somewhere else
	struct selected_segment_data
	{
		//TODO: maybe store the tag name instead of a pointer
		vt::tag* tag{};
		tag_timeline* segments;
		tag_timeline::iterator segment_it;
	};

	struct moving_segment_data
	{
		//TODO: maybe store the tag name instead of a pointer
		vt::tag* tag{};
		tag_timeline::iterator segment_it{};
		uint8_t grab_part{};
		timestamp grab_position{};
		timestamp start{};
		timestamp end{};
	};

	struct insert_segment_data
	{
		std::string tag;
		timestamp start{};
		timestamp end{};
		bool ready{};
		bool show_insert_popup{};
		bool show_merge_popup = true;

		//used in insert popup
		int name_index{};
	};

	using insert_segment_data_container = std::unordered_map<std::string, insert_segment_data>;

	struct timeline_state
	{
		static constexpr timestamp disabled_time_min = timestamp::zero();
		static constexpr timestamp disabled_time_max = timestamp{ 3600 };

		bool focused = false;
		bool enabled = true;

		tag_storage* tags{};
		segment_storage* segments{};

		video_group_id_t current_video_group_id = invalid_video_group_id;
		std::unordered_map<video_group_id_t, std::vector<std::string>> displayed_tags;

		timestamp time_min{};
		timestamp time_max{};

		int64_t first_frame{};

		timestamp current_time{};

		tag& get(size_t index);
		void add(const std::string& name);
		void del(size_t index);

		void sync_tags();
	};

	//Inspector needs this
	extern bool merge_segments_popup(const std::string& id, bool& pressed_button, bool display_dragged_segment_text);

	bool video_timeline(timeline_state& state, std::optional<selected_segment_data>& selected_timestamp, std::optional<moving_segment_data>& moving_timestamp, insert_segment_data_container& insert_segment_container, bool& dirty_flag);

}
