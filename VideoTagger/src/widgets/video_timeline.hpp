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

#include <utils/timestamp.hpp>
#include <tags/tag_storage.hpp>

namespace vt::widgets
{
	//TODO: maybe should be moved somewhere else
	struct selected_timestamp_data
	{
		//TODO: maybe store the tag name instead of a pointer
		vt::tag* tag{};
		tag_timeline* timestamp_timeline;
		tag_timeline::iterator timestamp;
	};

	struct moving_timestamp_data
	{
		//TODO: maybe store the tag name instead of a pointer
		vt::tag* tag{};
		tag_timeline::iterator segment{};
		uint8_t grab_part{};
		timestamp grab_position{};
		timestamp start{};
		timestamp end{};
	};

	struct timeline_state
	{
		bool focused = false;

		tag_storage* tags{};
		std::vector<std::string> displayed_tags;

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
	extern bool merge_timestamps_popup(const std::string& id, bool& pressed_button);
	
	extern bool insert_timestamp_popup(const std::string& id, tag& tag, timestamp& start, timestamp& end, uint64_t min_timestamp, uint64_t max_timestamp);

	// return true if selection is made
	bool video_timeline(timeline_state& state, std::optional<selected_timestamp_data>& selected_timestamp, std::optional<moving_timestamp_data>& moving_timestamp, bool& dirty_flag);

}
