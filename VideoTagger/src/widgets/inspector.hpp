#pragma once
#include <optional>
#include <limits>
#include "video_timeline.hpp"

namespace vt::widgets
{
	extern bool inspector(std::optional<selected_segment_data>& selected_segment, std::optional<moving_segment_data>& moving_segment, bool& link_start_end, bool& dirty_flags, bool* open = nullptr, uint64_t min_timestamp = 0, uint64_t max_timestamp = std::numeric_limits<uint64_t>::max());
}
