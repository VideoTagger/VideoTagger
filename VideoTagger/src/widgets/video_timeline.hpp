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
		std::optional<timestamp> start{};
		std::optional<timestamp> end{};
		bool ready{};
		bool show_insert_popup{};
		bool show_merge_popup = true;

		//used in insert popup
		int name_index = -1;
	};

	using insert_segment_data_container = std::unordered_map<std::string, insert_segment_data>;

	class video_timeline
	{
	public:
		static constexpr timestamp disabled_time_min = timestamp::zero();
		static constexpr timestamp disabled_time_max = timestamp{ 3600 };
	
		std::optional<selected_segment_data> selected_segment;
		std::optional<moving_segment_data> moving_segment;
		insert_segment_data_container* insert_segment_container{};

		void set_video_group_id(video_group_id_t id);
		video_group_id_t video_group_id() const;

		void set_tag_storage(tag_storage* tags);
		tag_storage* tag_storage() const;

		void set_segment_storage(segment_storage* segments);
		segment_storage* segment_storage() const;

		void set_enabled(bool value);
		bool is_enabled() const;

		void set_start_timestamp(timestamp ts);
		timestamp start_timestamp() const;

		void set_end_timestamp(timestamp ts);
		timestamp end_timestamp() const;

		void set_current_timestamp(timestamp ts);
		timestamp current_timestamp() const;

		void render(bool& open);

	private:
		bool focused_ = false;
		bool enabled_ = true;
		
		vt::tag_storage* tags_{};
		vt::segment_storage* segments_{};

		int64_t first_frame_{};

		timestamp time_min_{};
		timestamp time_max_{};
		timestamp current_time_{};

		video_group_id_t current_video_group_id_ = invalid_video_group_id;
	};

	//Inspector needs this
	extern bool merge_segments_popup(const std::string& id, bool& pressed_button, bool display_dragged_segment_text);

}
