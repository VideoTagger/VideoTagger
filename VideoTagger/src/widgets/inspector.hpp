#pragma once
#include <optional>
#include <limits>
#include "video_timeline.hpp"

namespace vt::widgets
{
	extern bool inspector(std::optional<selected_timestamp_data>& selected_timestamp, std::optional<moving_timestamp_data>& moving_timestamp, bool& dirty_flags, bool* open = nullptr, uint64_t min_timestamp = 0, uint64_t max_timestamp = std::numeric_limits<uint64_t>::max());
}
