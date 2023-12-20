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

#include <utils/video_time.hpp>

struct ImDrawList;
struct ImRect;
namespace ImSequencer
{
	enum SEQUENCER_OPTIONS
	{
		SEQUENCER_EDIT_NONE = 0,
		SEQUENCER_EDIT_STARTEND = 1 << 1,
		SEQUENCER_CHANGE_FRAME = 1 << 3,
		SEQUENCER_ADD = 1 << 4,
		SEQUENCER_DEL = 1 << 5,
		SEQUENCER_COPYPASTE = 1 << 6,
		SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
	};
}

namespace vt
{
	struct timeline_interface
	{
		bool focused = false;
		virtual video_time_t get_time_min() const = 0;
		virtual video_time_t get_time_max() const = 0;
		virtual int get_item_count() const = 0;

		virtual void begin_edit(int /*index*/) {}
		virtual void end_edit() {}
		virtual int get_item_type_count() const { return 0; }
		virtual const char* get_item_type_name(int /*typeIndex*/) const { return ""; }
		virtual const char* get_item_label(int /*index*/) const { return ""; }
		virtual const char* get_collapse_fmt() const { return "%d Frames / %d entries"; }

		virtual void get(int index, int** start, int** end, int* type, unsigned int* color) = 0;
		virtual void add(int /*type*/) {}
		virtual void del(int /*index*/) {}
		virtual void duplicate(int /*index*/) {}

		virtual void copy() {}
		virtual void paste() {}

		virtual size_t get_custom_height(int /*index*/) { return 0; }
		virtual void double_click(int /*index*/) {}
		virtual void custom_draw(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*legendRect*/, const ImRect& /*clippingRect*/, const ImRect& /*legendClippingRect*/) {}
		virtual void custom_draw_compact(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*clippingRect*/) {}

		virtual ~timeline_interface() = default;
	};


	// return true if selection is made
	bool video_timeline(timeline_interface* sequence, video_time_t* current_time, bool* expanded, int* selected_entry, int64_t* first_frame, int sequence_options);

}
